#include "execution/OrderExecutionSim.hpp"

#include "messageHandling/OrderBookLevel.hpp"

namespace execution {
    void OrderExecutionSim::submitOrder(const BuySell buySell, const double quantity, const double price) {
        positionManager.updatePositionWithTrade(buySell, quantity, price); // not trading, just simulating
    }

    void OrderExecutionSim::resetPosition(const OrderBookLevel orderBookLevel) {
        const double position = positionManager.getPosition();

        if (position == 0) return;
        const bool positionIsShort = position < 0;

        const double price = positionIsShort ? orderBookLevel.getBestBid().getPrice() : orderBookLevel.getBestAsk().getPrice();

        positionManager.updatePositionWithTrade(positionIsShort ? BuySell::BUY : BuySell::SELL, std::abs(position), price);
    }
}
