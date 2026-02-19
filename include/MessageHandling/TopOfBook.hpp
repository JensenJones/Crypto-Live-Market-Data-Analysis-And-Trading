#pragma once
#include <ostream>
#include <string>
#include <nlohmann/json_fwd.hpp>

#include "MarketOrder.hpp"

class TopOfBook {
    const uint64_t updateId;
    const std::string symbol;
    MarketOrder bestBid;
    MarketOrder bestAsk;

public:
    explicit TopOfBook(const nlohmann::json& marketMessage);

    TopOfBook(const TopOfBook &other) = default;

    TopOfBook(TopOfBook &&other) noexcept = default;

    TopOfBook(uint64_t updateId, std::string symbol, const MarketOrder &bestBid, const MarketOrder &bestAsk);

    friend std::ostream & operator<<(std::ostream &os, const TopOfBook &obj);

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
