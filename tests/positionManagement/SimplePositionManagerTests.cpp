#include <gtest/gtest.h>
#include "positionManagement/SimplePositionManager.hpp"

using namespace positionManagement;
using Order::BuySell;

class SimplePositionManagerTests : public testing::Test {
protected:
    std::unique_ptr<SimplePositionManager> subject;

    void SetUp() override {
        subject = std::make_unique<SimplePositionManager>("USDTBTC"); // start fresh each test
    }
};

// Buying into empty position
TEST_F(SimplePositionManagerTests, BuyIntoEmptyPosition) {
    subject->updatePositionWithTrade(BuySell::BUY, 10.0, 100.0);
    EXPECT_EQ(subject->getPosition(), 10.0);
    EXPECT_DOUBLE_EQ(subject->getAveragePrice(), 100.0);
    EXPECT_DOUBLE_EQ(subject->getRealisedPnl(), 0.0);
}

// Selling into empty position
TEST_F(SimplePositionManagerTests, SellIntoEmptyPosition) {
    subject->updatePositionWithTrade(BuySell::SELL, 5.0, 200.0);
    EXPECT_EQ(subject->getPosition(), -5.0);
    EXPECT_DOUBLE_EQ(subject->getAveragePrice(), 200.0);
    EXPECT_DOUBLE_EQ(subject->getRealisedPnl(), 0.0);
}

// Adding to same direction
TEST_F(SimplePositionManagerTests, AddToExistingPosition) {
    subject->updatePositionWithTrade(BuySell::BUY, 10.0, 100.0);
    subject->updatePositionWithTrade(BuySell::BUY, 5.0, 120.0);

    EXPECT_EQ(subject->getPosition(), 15.0);
    EXPECT_DOUBLE_EQ(subject->getAveragePrice(), (10*100 + 5*120)/15.0);
    EXPECT_DOUBLE_EQ(subject->getRealisedPnl(), 0.0);
}

// Reducing position partially
TEST_F(SimplePositionManagerTests, ReducePositionPartially) {
    subject->updatePositionWithTrade(BuySell::BUY, 10.0, 100.0);
    subject->updatePositionWithTrade(BuySell::SELL, 4.0, 110.0);

    EXPECT_EQ(subject->getPosition(), 6.0);
    EXPECT_DOUBLE_EQ(subject->getAveragePrice(), 100.0); // averagePrice unchanged
    EXPECT_DOUBLE_EQ(subject->getRealisedPnl(), 4.0 * (110 - 100)); // 40.0
}

// Closing position fully
TEST_F(SimplePositionManagerTests, ClosePositionFully) {
    subject->updatePositionWithTrade(BuySell::BUY, 10.0, 100.0);
    subject->updatePositionWithTrade(BuySell::SELL, 10.0, 105.0);

    EXPECT_EQ(subject->getPosition(), 0.0);
    EXPECT_DOUBLE_EQ(subject->getAveragePrice(), 0.0);
    EXPECT_DOUBLE_EQ(subject->getRealisedPnl(), 10.0 * (105 - 100)); // 50.0
}

// Flipping from long to short
TEST_F(SimplePositionManagerTests, FlipPositionLongToShort) {
    subject->updatePositionWithTrade(BuySell::BUY, 10.0, 100.0);
    subject->updatePositionWithTrade(BuySell::SELL, 15.0, 110.0);

    EXPECT_EQ(subject->getPosition(), -5.0);
    EXPECT_DOUBLE_EQ(subject->getAveragePrice(), 110.0); // new short average price
    EXPECT_DOUBLE_EQ(subject->getRealisedPnl(), 10.0 * (110 - 100)); // profit from closing long
}

// Flipping from short to long
TEST_F(SimplePositionManagerTests, FlipPositionShortToLong) {
    subject->updatePositionWithTrade(BuySell::SELL, 8.0, 200.0);
    subject->updatePositionWithTrade(BuySell::BUY, 10.0, 190.0);

    EXPECT_EQ(subject->getPosition(), 2.0);
    EXPECT_DOUBLE_EQ(subject->getAveragePrice(), 190.0); // new long average price
    EXPECT_DOUBLE_EQ(subject->getRealisedPnl(), 8.0 * (200 - 190)); // profit from closing short
}