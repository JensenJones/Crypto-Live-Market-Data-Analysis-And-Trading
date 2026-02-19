#pragma once

#include <ostream>
#include <utility>

namespace Order {
    enum class BuySell {
        BUY,
        SELL,
    };

    inline std::ostream& operator<<(std::ostream& os, const BuySell& buySell) {
        switch (buySell) {
            case BuySell::BUY: return os << "BUY";
            case BuySell::SELL: return os << "SELL";
            default: std::unreachable();
        }
    }
}