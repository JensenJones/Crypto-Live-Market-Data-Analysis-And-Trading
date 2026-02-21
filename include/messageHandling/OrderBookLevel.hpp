#pragma once
#include <ostream>
#include <string>
#include <nlohmann/json_fwd.hpp>

#include "MarketOrder.hpp"

class OrderBookLevel {
    const uint64_t updateId;
    const std::string symbol;
    MarketOrder bestBid;
    MarketOrder bestAsk;

public:
    explicit OrderBookLevel(const nlohmann::json& marketMessage);

    OrderBookLevel(const OrderBookLevel &other) = default;

    OrderBookLevel(OrderBookLevel &&other) noexcept = default;

    OrderBookLevel(uint64_t updateId, std::string symbol, const MarketOrder &bestBid, const MarketOrder &bestAsk);

    friend std::ostream & operator<<(std::ostream &os, const OrderBookLevel &obj);

    [[nodiscard]] uint64_t getUpdateId() const {
        return updateId;
    }

    [[nodiscard]] std::string getSymbol() const {
        return symbol;
    }

    [[nodiscard]] MarketOrder getBestBid() const {
        return bestBid;
    }

    [[nodiscard]] MarketOrder getBestAsk() const {
        return bestAsk;
    }
};
