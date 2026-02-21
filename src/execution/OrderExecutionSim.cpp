#include "execution/OrderExecutionSim.hpp"

#include "messageHandling/OrderBookLevel.hpp"

namespace execution {
    void OrderExecutionSim::submitOrder(const BuySell buySell, const double quantity, const double price) {
        positionManager.updatePositionWithTrade(buySell, quantity, price); // not trading, just simulating
    }

    void OrderExecutionSim::resetPosition(const OrderBookLevel orderBookLevel) {
        const double position = positionManager.getPosition();

        if (position == 0) return;

        const double price = position < 0 ? orderBookLevel.getBestBid().getPrice() : orderBookLevel.getBestAsk().getPrice();

        positionManager.updatePositionWithTrade(position < 0 ? BuySell::BUY : BuySell::SELL, position, price);
    }
}