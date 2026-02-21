#include <gtest/gtest.h>

#include "tradeData/SignalEngine.hpp"
#include "tradeData/metrics/BidAskVolumeRatio.hpp"
#include <gmock/gmock.h>

#include "messageHandling/MarketOrder.hpp"
#include "messageHandling/OrderBookLevel.hpp"
#include "tradeData/metrics/MetricNames.hpp"

#include <memory>

using sigEng = tradeData::SignalEngine;
using Order::BuySell;

namespace {

    class MockMetric : public tradeData::metrics::Metric {
    public:
        MOCK_METHOD(void, update, (const OrderBookLevel&), (override));
        MOCK_METHOD(double, getMetric, (), (const, override));
    };

    class MockPositionManager : public positionManagement::PositionManager {
    public:
        explicit MockPositionManager(const std::string& symbol)
            : PositionManager(symbol) {}
        MOCK_METHOD(void, updatePositionWithTrade,
            (Order::BuySell side, double quantity, double price),
            (override));
    };

    class MockOrderExecution : public execution::OrderExecution {
    public:
        explicit MockOrderExecution(execution::PM& pm)
            : OrderExecution(pm) {}

        MOCK_METHOD(void, submitOrder,
                    (BuySell side, double quantity, double price),
                    (override));
    };

}

class SignalEngineTests : public testing::Test {
protected:
    std::unique_ptr<MockPositionManager> mockPm;
    std::unique_ptr<MockOrderExecution> mockExecutor;
    std::unique_ptr<sigEng> subject;
    const std::string symbol = "BTC/USDT";

    void SetUp() override {
        mockPm = std::make_unique<MockPositionManager>(symbol);
        mockExecutor = std::make_unique<MockOrderExecution>(*mockPm);
        subject = std::make_unique<sigEng>(symbol, *mockExecutor);
    }
};

TEST_F(SignalEngineTests, shouldAddAndRemoveMetric) {
    auto mock = std::make_unique<MockMetric>();

    EXPECT_NO_THROW(
        subject->addMetric(MetricName::BID_ASK_VOLUME_RATIO, std::move(mock), {0.0, 1.0})
    );

    auto result = subject->removeMetric(MetricName::BID_ASK_VOLUME_RATIO);

    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result.value());
}

TEST_F(SignalEngineTests, shouldNotSubmitOrderForStaleUpdate) {
    auto mockMetric = std::make_unique<MockMetric>();
    auto* metricPtr = mockMetric.get();

    subject->addMetric(MetricName::BID_ASK_VOLUME_RATIO,
                       std::move(mockMetric),
                       {0.5, 2.0});

    const OrderBookLevel fresh{
        5, "BTC/USDT",
        MarketOrder{99.5, 1.0},
        MarketOrder{100.5, 1.0}
    };

    const OrderBookLevel stale{
        3, "BTC/USDT",
        MarketOrder{99.5, 1.0},
        MarketOrder{100.5, 1.0}
    };

    EXPECT_CALL(*metricPtr, update(testing::_)).Times(1);
    EXPECT_CALL(*metricPtr, getMetric())
        .WillOnce(testing::Return(4.0));

    EXPECT_CALL(*mockExecutor, submitOrder(BuySell::BUY, 1, 99.5)).Times(1);

    subject->processData(fresh);
    subject->processData(stale);
}

TEST_F(SignalEngineTests, shouldThrowWhenAddingDuplicateMetric) {
    auto mock1 = std::make_unique<MockMetric>();
    auto mock2 = std::make_unique<MockMetric>();

    subject->addMetric(MetricName::BID_ASK_VOLUME_RATIO, std::move(mock1), {0.5, 2.0});

    EXPECT_THROW(
        subject->addMetric(MetricName::BID_ASK_VOLUME_RATIO, std::move(mock2), {0.5, 2.0}),
        std::runtime_error
    );
}

TEST_F(SignalEngineTests, shouldReturnErrorWhenRemovingNonExistentMetric) {
    auto result = subject->removeMetric(MetricName::BID_ASK_VOLUME_RATIO);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), "Metric never added");
}

TEST_F(SignalEngineTests, shouldSubmitBuyOrderWhenAllMetricsExceedBuyLimit) {
    auto mockMetric = std::make_unique<MockMetric>();
    auto* metricPtr = mockMetric.get();

    subject->addMetric(MetricName::BID_ASK_VOLUME_RATIO,
                       std::move(mockMetric),
                       {0.5, 2.0});

    const OrderBookLevel tob{
        1,
        "BTC/USDT",
        MarketOrder{99.5, 4.0},   // best bid
        MarketOrder{100.5, 1.0}   // best ask
    };

    EXPECT_CALL(*metricPtr, update(testing::_)).Times(1);
    EXPECT_CALL(*metricPtr, getMetric())
        .WillOnce(testing::Return(4.0)); // > buyLimit (2.0)

    EXPECT_CALL(*mockExecutor, submitOrder(BuySell::BUY, 1, 99.5)).Times(1);

    subject->processData(tob);
}

TEST_F(SignalEngineTests, shouldSubmitSellOrderWhenAllMetricsBelowSellLimit) {
    auto mockMetric = std::make_unique<MockMetric>();
    auto* metricPtr = mockMetric.get();

    subject->addMetric(MetricName::BID_ASK_VOLUME_RATIO,
                       std::move(mockMetric),
                       {0.5, 2.0});

    const OrderBookLevel tob{
        1,
        "BTC/USDT",
        MarketOrder{99.5, 1.0},
        MarketOrder{100.5, 1.0}   // best ask
    };

    EXPECT_CALL(*metricPtr, update(testing::_)).Times(1);
    EXPECT_CALL(*metricPtr, getMetric())
        .WillOnce(testing::Return(0.2)); // < sellLimit (0.5)

    EXPECT_CALL(*mockExecutor, submitOrder(BuySell::SELL, 1, 100.5)).Times(1);

    subject->processData(tob);
}

TEST_F(SignalEngineTests, shouldNotSubmitOrderWhenSignalsConflict) {
    auto mock1 = std::make_unique<MockMetric>();
    auto mock2 = std::make_unique<MockMetric>();
    auto* ptr1 = mock1.get();
    auto* ptr2 = mock2.get();

    subject->addMetric(MetricName::BID_ASK_VOLUME_RATIO,
                       std::move(mock1),
                       {0.5, 2.0});

    subject->addMetric(MetricName::MID_PRICE_REALISED_VOLATILITY,
                       std::move(mock2),
                       {0.1, 2.0});

    const OrderBookLevel tob{
        1,
        "BTC/USDT",
        MarketOrder{99.5, 1.0},
        MarketOrder{100.5, 1.0}
    };

    EXPECT_CALL(*ptr1, update(testing::_)).Times(1);
    EXPECT_CALL(*ptr2, update(testing::_)).Times(1);

    EXPECT_CALL(*ptr1, getMetric())
        .WillOnce(testing::Return(4.0));   // BUY signal

    EXPECT_CALL(*ptr2, getMetric())
        .WillOnce(testing::Return(0.01));  // SELL signal

    EXPECT_CALL(*mockExecutor, submitOrder(testing::_, testing::_, testing::_)).Times(0);

    subject->processData(tob);
}

TEST_F(SignalEngineTests, shouldChangeBuySellDecisionAfterDifferentTopOfOrderBooks) {
    auto mockMetric = std::make_unique<MockMetric>();
    auto* metricPtr = mockMetric.get();

    // Add a metric with buy/sell thresholds
    subject->addMetric(MetricName::BID_ASK_VOLUME_RATIO,
                       std::move(mockMetric),
                       {0.5, 2.0});

    // First top-of-book triggers a BUY signal
    const OrderBookLevel tobBuy{
        1,
        "BTC/USDT",
        MarketOrder{99.5, 2.0},  // best bid
        MarketOrder{100.5, 1.0}  // best ask
    };

    // Second top-of-book triggers a SELL signal
    const OrderBookLevel tobSell{
        2,
        "BTC/USDT",
        MarketOrder{99.5, 1.0},  // best bid
        MarketOrder{101.0, 3.0}  // best ask
    };

    // Expect metric update to be called for each order book
    EXPECT_CALL(*metricPtr, update(testing::_)).Times(2);

    // First metric value -> BUY (above buyLimit 2.0)
    EXPECT_CALL(*metricPtr, getMetric())
        .WillOnce(testing::Return(4.0))
        // Second metric value -> SELL (below sellLimit 0.5)
        .WillOnce(testing::Return(0.2));

    // Expect submitOrder to be called with BUY first, then SELL
    {
        testing::InSequence seq; // ensure order
        EXPECT_CALL(*mockExecutor, submitOrder(BuySell::BUY, 1, 99.5));
        EXPECT_CALL(*mockExecutor, submitOrder(BuySell::SELL, 1, 101.0));
    }

    // Process both top-of-book updates
    subject->processData(tobBuy);
    subject->processData(tobSell);
}