#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/strand.hpp>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <boost/asio/signal_set.hpp>
#include <thread>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <nlohmann/json.hpp>
#include "webConnection/WebSocketClient.cpp"

namespace beast = boost::beast; // from <boost/beast.hpp>
namespace http = beast::http; // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio; // from <boost/asio.hpp>
namespace ssl = boost::asio::ssl; // from <boost/asio/ssl.hpp>
using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>
typedef boost::asio::io_context::executor_type executor_type;
typedef boost::asio::strand<executor_type> strand;

int main(int argc, char **argv) {
    // Check command line arguments.
    if (argc != 7) {
        std::cerr <<
                "Usage: websocket-client-async-ssl <host> <port> <text> <num consumer threads>\n" <<
                "Example:\n" <<
                "    stream.binance.com 9443 '{ \"method\": \"SUBSCRIBE\", \"params\": [ \"btcusdt@aggTrade\", \"btcusdt@depth\" ], \"id\": 1 }' \"/stream?streams=btcusdt@trade&timeUnit=MILLISECOND\" 1\n";
        return EXIT_FAILURE;
    }

    // TODO: look into the 'Individual Symbol Rolling Window Statistics Streams'

    auto const host = argv[1];
    auto const port = argv[2];
    auto const text = argv[3];
    auto const endpoint = argv[4];
    const int numConsumers = std::stoi(argv[6]);
    auto const threads = std::max<size_t>(1, static_cast<size_t>(std::strtol(argv[5], nullptr, 10)));

    // The io_context is required for all I/O
    net::io_context ioc;

    // Signal handler to exit cleanly
    boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);
    signals.async_wait([&ioc](auto, auto) {
        ioc.stop();
        std::cout << "Signal received, stopped the process.";
    });

    // The SSL context is required, and holds certificates
    ssl::context ctx{ssl::context::tlsv12_client};

    // This holds the root certificate used for verification
    ctx.set_default_verify_paths();
    ctx.set_verify_mode(ssl::verify_peer);

    // Launch the asynchronous operation
    // Create an instance of session and returns a shared  pointer to session
    std::make_shared<session>(ioc, ctx)->run(host, port, text, endpoint, numConsumers);

    // Run the I/O service on the requested number of threads
    std::vector<std::thread> v;
    v.reserve(threads - 1);
    for (auto i = 1; i < threads; ++i)
        v.emplace_back([&ioc] {ioc.run();});

    ioc.run();

    // (If we get here, it means we got a SIGINT or SIGTERM)
    // Block until all the threads exit
    for (auto &t: v)
        t.join();

    return EXIT_SUCCESS;
}
