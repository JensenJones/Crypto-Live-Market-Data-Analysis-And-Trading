#include "../../include/MessageHandling/OrderFactory.hpp"

std::optional<MarketOrder> OrderFactory::newData(double newData) {
    if (!hasPrice) {
        price = newData;
        hasPrice = true;
        return std::nullopt;
    }
    hasPrice = false;
    return std::make_optional<MarketOrder>(MarketOrder(price, newData));
}
