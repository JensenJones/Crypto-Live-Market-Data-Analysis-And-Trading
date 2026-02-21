#include "execution/OrderExecutionSim.hpp"

namespace execution {
    void OrderExecutionSim::submitOrder(const BuySell buySell, const double quantity, const double price) {
        positionManager.updatePositionWithTrade(buySell, quantity, price); // not trading, just simulating
    }
}
