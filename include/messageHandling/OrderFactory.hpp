#pragma once
#include <optional>

#include "MarketOrder.hpp"

class OrderFactory {
    bool hasPrice{};
    double price{};
    MarketOrder order;

public:
    std::optional<MarketOrder> newData(double newData);
};
