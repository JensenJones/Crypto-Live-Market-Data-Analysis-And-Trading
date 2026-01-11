/**
 * TODO:
 * Create simple market metrics
 * Interface for combining the metrics and trading based on boolean combination of them
 * Interchangeability of markets while running.
 **/

#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/strand.hpp>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <cstdio>
#include <thread>
#include <nlohmann/adl_serializer.hpp>
#include <nlohmann/json.hpp>
#include <openssl/ssl.h>

#include "../../include/MessageQueue/MessageQueue.hpp"
#include "../../include/MessageQueue/MessageQueueConsumer.hpp"
#include "../../include/MessageHandling/TopOfBook.hpp"
#include "../../include/DataProcessing/Metrics/BidAskVolumeRatio.hpp"


namespace beast = boost::beast; // from <boost/beast.hpp>
namespace http = beast::http; // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio; // from <boost/asio.hpp>
namespace ssl = boost::asio::ssl; // from <boost/asio/ssl.hpp>
using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>
typedef boost::asio::io_context::executor_type executor_type;
typedef boost::asio::strand<executor_type> strand;

//------------------------------------------------------------------------------

// Report a failure
void
fail(beast::error_code ec, char const *what) {
    std::cerr << what << ": " << ec.message() << "\n";
}

// Sends a WebSocket message and prints the response
// enabled_shared_from_this provided function shared_from_this
// which is a handy function to create shared pointers from this
class session : public std::enable_shared_from_this<session> {
    constexpr static size_t TOB_QUEUE_MAX_SIZE = 1000;
    constexpr static size_t ORDER_QUEUE_MAX_SIZE = 1000;

    net::io_context &ioc_;
    tcp::resolver resolver_;
    websocket::stream<beast::ssl_stream<beast::tcp_stream> > ws_;
    beast::flat_buffer buffer_;
    std::string host_;
    std::string text_;
    std::string endpoint_;
    strand ws_strand_;

    using TobMessageQueue = messageQueue::MessageQueue<TopOfBook, TOB_QUEUE_MAX_SIZE>;
    using OrderMessageQueue = messageQueue::MessageQueue<Order, ORDER_QUEUE_MAX_SIZE>;
    using DataProcessor = dataProcessing::DataProcessor<OrderMessageQueue>;
    using TobQueueConsumer = messageQueue::MessageQueueConsumer<TobMessageQueue, DataProcessor>;

    TobMessageQueue tobMessageQueue_{};
    std::vector<std::unique_ptr<TobQueueConsumer>> tobMqConsumers;
    std::vector<std::jthread> tobMqConsumerThreads_;

    OrderMessageQueue orderMessageQueue_{};
    DataProcessor dataProcessor_{"btcusdt", orderMessageQueue_}; // TODO: Make symbol dynamic

    void startConsumers(const int n) {
        tobMqConsumers.reserve(n);
        tobMqConsumerThreads_.reserve(n);

        for (int i = 0; i < n; ++i) {
            tobMqConsumers.emplace_back(std::make_unique<TobQueueConsumer>(tobMessageQueue_, dataProcessor_));
            tobMqConsumerThreads_.emplace_back(std::ref(*tobMqConsumers.back()));
        }
    }

public:
    // Resolver and socket require an io_context
    explicit
    session(net::io_context &ioc, ssl::context &ctx)
        : ioc_(ioc) // Looks up the domain name
          , resolver_(net::make_strand(ioc)) // Websocket constructor takes an IO context and ssl context
          , ws_(net::make_strand(ioc), ctx) // reference to the io_context created in the main function
          , ws_strand_(ioc.get_executor()) // Get a strand from the io_context
    {
    }

    // Start the asynchronous operation
    void
    run(
        char const *host,
        char const *port,
        char const *text,
        char const *endpoint,
        const int numConsumers) {
        startConsumers(numConsumers);
        dataProcessor_.addMetric(MetricName::BID_ASK_VOLUME_RATIO,
            std::make_unique<dataProcessing::metrics::BidAskVolumeRatio>(40));

        // Save these for later
        host_ = host;
        text_ = text;
        endpoint_ = endpoint;

        // Look up the domain name
        resolver_.async_resolve(
            host,
            port,
            // Binds parameters to the handler creating a new handler
            beast::bind_front_handler(
                &session::on_resolve,
                shared_from_this()));
    }

    void
    on_resolve(
        const beast::error_code &errorCode,
        const tcp::resolver::results_type &results) {
        if (errorCode)
            return fail(errorCode, "resolve");

        // Set a timeout on the operation
        // The lowest layer will get the underlying socket
        //websocket::stream<beast::ssl_stream<beast::tcp_stream>> ws_;
        beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(30l));

        // Make the connection on the IP address we get from a lookup
        beast::get_lowest_layer(ws_).async_connect(
            results,
            // Binds the strand to an executor of the type the strand is based on, in this case io_context
            // bind_front_handler creates a functor object which when execute calls on_connect function of this pointer i.e
            // current instance of session.
            // Once Async_connect completes it calls the functor object with error_code errorCode and endpoint_type ep
            // which are required arguments to call on_connect function
            boost::asio::bind_executor(
                ws_strand_, beast::bind_front_handler(&session::on_connect, shared_from_this())));
        // beast::bind_front_handler(
        //     &session::on_connect,
        //     shared_from_this()));
    }

    void
    on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type ep) {
        if (ec)
            return fail(ec, "connect");

        // Gets the socket associated with this web socket and sets a timeout on the operation.
        // ws_ is declared as websocket::stream<beast::ssl_stream<beast::tcp_stream>>
        // get_lower_layer will return beast::tcp_stream
        beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(30ll));

        // Set SNI Hostname (many hosts need this to handshake successfully)
        if (!SSL_set_tlsext_host_name(
            ws_.next_layer().native_handle(),
            host_.c_str())) {
            ec = beast::error_code(static_cast<int>(::ERR_get_error()),
                                   net::error::get_ssl_category());
            return fail(ec, "connect");
        }

        // Update the host_ string. This will provide the value of the
        // Host HTTP header during the WebSocket handshake.
        // See https://tools.ietf.org/html/rfc7230#section-5.4
        host_ += ':' + std::to_string(ep.port());

        // Perform the SSL handshake.
        // ws_ is declared as websocket::stream<beast::ssl_stream<beast::tcp_stream>>
        // next_layer will return beast::ssl_stream
        ws_.next_layer().async_handshake(
            ssl::stream_base::client,
            boost::asio::bind_executor(
                ws_strand_, beast::bind_front_handler(&session::on_ssl_handshake, shared_from_this())));
    }

    void
    on_ssl_handshake(beast::error_code ec) {
        if (ec)
            return fail(ec, "ssl_handshake");

        // Turn off the timeout on the tcp_stream, because the websocket stream has its own timeout system.
        beast::get_lowest_layer(ws_).expires_never();

        // Set suggested timeout settings for the websocket
        ws_.set_option(
            websocket::stream_base::timeout::suggested(
                beast::role_type::client));

        // Set a decorator to change the User-Agent of the handshake
        ws_.set_option(websocket::stream_base::decorator(
            [](websocket::request_type &req) {
                req.set(http::field::user_agent,
                        std::string(BOOST_BEAST_VERSION_STRING) +
                        " websocket-client-async-ssl");
            }));

        // Perform the websocket handshake
        ws_.async_handshake(host_, endpoint_,
                            boost::asio::bind_executor(
                                ws_strand_, beast::bind_front_handler(&session::on_handshake, shared_from_this())));
    }

    void
    on_handshake(beast::error_code ec) {
        if (ec)
            return fail(ec, "handshake");

        // Send the message
        ws_.async_write(
            net::buffer(text_),
            boost::asio::bind_executor(ws_strand_, beast::bind_front_handler(&session::on_write, shared_from_this())));
    }

    void
    on_write(
        beast::error_code ec,
        std::size_t bytes_transferred) {
        // Supress unused variable compiler warnings
        // bytes_transferred contains total bytes read/written to the websocket stream
        boost::ignore_unused(bytes_transferred);

        if (ec)
            return fail(ec, "write");

        // Read single message
        ws_.async_read(
            buffer_,
            boost::asio::bind_executor(
                ws_strand_, beast::bind_front_handler(&session::on_read, shared_from_this())));
    }

    void on_read(beast::error_code ec, std::size_t bytes_transferred) {
        boost::ignore_unused(bytes_transferred);
        if (ec) return fail(ec, "read");

        if (std::string message = beast::buffers_to_string(buffer_.data()); message.length() > 30) { // Avoid invalid messages
            tobMessageQueue_.enqueue(std::move(TopOfBook{nlohmann::json::parse(message)}));
        }

        buffer_.consume(buffer_.size());

        ws_.async_read(
            buffer_,
            boost::asio::bind_executor(
                ws_strand_,
                beast::bind_front_handler(&session::on_read, shared_from_this())
            )
        );
    }


    // Check how to close when a signal handler is triggered? does the websocket auto close?
    void
    on_close(beast::error_code ec) {
        if (ec)
            return fail(ec, "close");

        // If we get here then the connection is closed gracefully
        // The make_printable() function helps print a ConstBufferSequence
        std::cout << beast::make_printable(buffer_.data()) << std::endl;
    }
};