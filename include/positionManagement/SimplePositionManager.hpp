#pragma once
#include "PositionManager.hpp"

namespace positionManagement {
    class SimplePositionManager final : public PositionManager {
    public:
        explicit SimplePositionManager(const std::string &symbol) : PositionManager(symbol) {}

        void updatePositionWithTrade(BuySell buySell, double quantity, double price) override;
    };
}

