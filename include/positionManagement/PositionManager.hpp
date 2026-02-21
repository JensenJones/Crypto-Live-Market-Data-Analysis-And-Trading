#pragma once
#include <utility>

#include "order/BuySell.hpp"

namespace positionManagement {
    using BuySell = Order::BuySell;

    class PositionManager {
    protected:
        std::string symbol;

        double position{};
        double averagePrice{};
        double realisedPnl{};

        explicit PositionManager(std::string symbol) : symbol(std::move(symbol)) {}

    public:
        virtual ~PositionManager() = default;
        virtual void updatePositionWithTrade(BuySell buySell, double quantity, double price) = 0;

        [[nodiscard]] double getPosition() const;
        [[nodiscard]] double getRealisedPnl() const;
        [[nodiscard]] double getAveragePrice() const;
    };
}