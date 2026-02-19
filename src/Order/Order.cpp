#include "../../include/Order/Order.hpp"

#include <utility>

Order::Order::Order(std::string symbol, const BuySell side, const OrderType type, const OrderExpiry expiry,
                           const long double quantity, const long double price,
                           const uint64_t recvWindow, const uint64_t timestamp): symbol(std::move(symbol)),
                                                                                 side(side),
                                                                                 type(type),
                                                                                 expiry(expiry),
                                                                                 quantity(quantity),
                                                                                 price(price),
                                                                                 recvWindow(recvWindow),
                                                                                 timestamp(timestamp) {
}

std::string Order::Order::convertToPayload() {
    // TODO https://developers.binance.com/docs/fiat/general-info#place-new-order-trade
    return "";
}
