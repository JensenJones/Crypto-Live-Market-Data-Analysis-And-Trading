#pragma once

#include <string>

#include "OrderExpiry.hpp"
#include "BuySell.hpp"
#include "OrderType.hpp"

// Payload Requirements (fiat) - https://developers.binance.com/docs/fiat/general-info#place-new-order-trade
// ------------------
// Parameter	    Value
// symbol	        LTCBTC
// side	            BUY
// type	            LIMIT
// timeInForce	    GTC
// quantity	        1
// price	        0.1
// recvWindow	    5000
// timestamp	    1499827319559

namespace Order {
    class Order {
    public:
        Order(std::string symbol, BuySell side, OrderType type, OrderExpiry expiry,
              long double quantity, long double price, uint64_t recvWindow, uint64_t timestamp);

    private:
        const std::string symbol;
        const BuySell side;
        const OrderType type;
        const OrderExpiry expiry;
        const long double quantity;
        const long double price;
        const uint64_t recvWindow;
        const uint64_t timestamp;

    public:
        std::string convertToPayload();
    };
}

