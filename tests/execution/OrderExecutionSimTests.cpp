#include <gtest/gtest.h>

#include "execution/OrderExecutionSim.hpp"
#include "positionManagement/PositionManager.hpp"
#include "messageHandling/OrderBookLevel.hpp"
#include "messageHandling/MarketOrder.hpp"

using execution::OrderExecutionSim;
using positionManagement::PositionManager;
using Order::BuySell;

// --------------------------------------------------
// Minimal concrete PositionManager for testing
// --------------------------------------------------

class TestPositionManager : public PositionManager {
public:
    TestPositionManager()
        : PositionManager("TEST") {}

    void updatePositionWithTrade(BuySell side,
                                 double quantity,
                                 double /*price*/) override
    {
        if (side == BuySell::BUY)
            position += quantity;
        else
            position -= quantity;
    }
};



static OrderBookLevel makeBook(double bidPrice, double askPrice)
{
    MarketOrder bid(bidPrice, 1.0);
    MarketOrder ask(askPrice, 1.0);

    return OrderBookLevel(
        1,
        "TEST",
        bid,
        ask
    );
}


TEST(OrderExecutionSimTest, SubmitOrderUpdatesPosition)
{
    TestPositionManager pm;
    OrderExecutionSim sim(pm);

    sim.submitOrder(BuySell::BUY, 5.0, 100.0);

    EXPECT_DOUBLE_EQ(pm.getPosition(), 5.0);
}

TEST(OrderExecutionSimTest, ResetDoesNothingWhenFlat)
{
    TestPositionManager pm;
    OrderExecutionSim sim(pm);

    const auto book = makeBook(99.0, 101.0);

    sim.resetPosition(book);

    EXPECT_DOUBLE_EQ(pm.getPosition(), 0.0);
}

TEST(OrderExecutionSimTest, ResetClosesLongPosition)
{
    TestPositionManager pm;
    OrderExecutionSim sim(pm);

    sim.submitOrder(BuySell::BUY, 10.0, 100.0);
    EXPECT_DOUBLE_EQ(pm.getPosition(), 10.0);

    const auto book = makeBook(99.0, 105.0);

    sim.resetPosition(book);

    EXPECT_DOUBLE_EQ(pm.getPosition(), 0.0);
}

TEST(OrderExecutionSimTest, ResetClosesShortPosition)
{
    TestPositionManager pm;
    OrderExecutionSim sim(pm);

    sim.submitOrder(BuySell::SELL, 7.0, 100.0);
    EXPECT_DOUBLE_EQ(pm.getPosition(), -7.0);

    const auto book = makeBook(95.0, 101.0);

    sim.resetPosition(book);

    EXPECT_DOUBLE_EQ(pm.getPosition(), 0.0);
}