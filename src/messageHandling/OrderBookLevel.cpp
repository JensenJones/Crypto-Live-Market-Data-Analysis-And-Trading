#include "../../include/messageHandling/OrderBookLevel.hpp"

#include <utility>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

double as_double(const json& j, const std::string& key) {
    return std::stod(j.at(key).get<std::string>());
}

OrderBookLevel::OrderBookLevel(const json& marketMessage) : updateId( marketMessage.at("u").get<int64_t>() ),
                                         symbol( marketMessage.at("s").get<std::string>()){
    bestBid.set_price(as_double(marketMessage, "b"));
    bestBid.set_quantity(as_double(marketMessage, "B"));
    bestAsk.set_price(as_double(marketMessage, "a"));
    bestAsk.set_quantity(as_double(marketMessage, "A"));
}

OrderBookLevel::OrderBookLevel(const uint64_t updateId, std::string symbol, const MarketOrder &bestBid, const MarketOrder &bestAsk): updateId(updateId),
                                                                                                               symbol(std::move(symbol)),
                                                                                                               bestBid(bestBid),
                                                                                                               bestAsk(bestAsk) {
}

std::ostream & operator<<(std::ostream &os, const OrderBookLevel &obj) {
    return os
           << "updateId: " << obj.updateId << ", symbol: " << obj.symbol << '\n'
           << "Top of order book: " << obj.bestBid << " | " << obj.bestAsk;
}
