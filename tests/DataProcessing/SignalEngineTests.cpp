#include <gtest/gtest.h>

#include "tradeData/SignalEngineTests.hpp"
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

}

class SignalEngineTests : public testing::Test {
protected:
    std::unique_ptr<sigEng> subject;

    void SetUp() override {
        subject = std::make_unique<sigEng>("BTC/USDT");
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

TEST_F(SignalEngineTests, shouldIndicateBuyWhenExceedingBuyLimit) {
    auto mock = std::make_unique<MockMetric>();
    auto* mockPtr = mock.get();

    subject->addMetric(MetricName::BID_ASK_VOLUME_RATIO, std::move(mock), {0.5, 2.0});

    const OrderBookLevel tob{1, "BTC/USDT", MarketOrder{99.5, 4.0}, MarketOrder{100.5, 1.0}};

    EXPECT_CALL(*mockPtr, update(testing::_)).Times(1);
    EXPECT_CALL(*mockPtr, getMetric()).WillOnce(testing::Return(4.0));

    subject->processData(tob);
}

TEST_F(SignalEngineTests, shouldIndicateSellWhenExceedingSellLimit) {
    auto mock = std::make_unique<MockMetric>();
    auto* mockPtr = mock.get();

    subject->addMetric(MetricName::BID_ASK_VOLUME_RATIO, std::move(mock), {0.5, 2.0});

    const OrderBookLevel tob{1, "BTC/USDT", MarketOrder{99.5, 0.2}, MarketOrder{100.5, 1.0}};

    EXPECT_CALL(*mockPtr, update(testing::_)).Times(1);
    EXPECT_CALL(*mockPtr, getMetric()).WillOnce(testing::Return(0.2));

    subject->processData(tob);
}

TEST_F(SignalEngineTests, shouldntPlaceOrderWhenMixedSignals) {
    auto mock1 = std::make_unique<MockMetric>();
    auto mock2 = std::make_unique<MockMetric>();
    auto* mockPtr1 = mock1.get();
    auto* mockPtr2 = mock2.get();

    subject->addMetric(MetricName::BID_ASK_VOLUME_RATIO, std::move(mock1), {0.5, 2.0});
    subject->addMetric(MetricName::MID_PRICE_REALISED_VOLATILITY, std::move(mock2), {0.1, 2.0});

    const OrderBookLevel tob{1, "BTC/USDT", MarketOrder{99.5, 1.0},
        MarketOrder{100.5, 1.0}};

    EXPECT_CALL(*mockPtr1, update(testing::_)).Times(1);
    EXPECT_CALL(*mockPtr2, update(testing::_)).Times(1);

    EXPECT_CALL(*mockPtr1, getMetric()).WillOnce(testing::Return(4.0)); // < 2... should indicate BUY
    EXPECT_CALL(*mockPtr2, getMetric()).WillOnce(testing::Return(0.01)); // < 0.1... should indicate SELL

    subject->processData(tob);
}

TEST_F(SignalEngineTests, shouldNotProcessStaleUpdate) {
    auto mock = std::make_unique<MockMetric>();
    auto* mockPtr = mock.get();

    subject->addMetric(MetricName::BID_ASK_VOLUME_RATIO, std::move(mock), {0.5, 2.0});

    const OrderBookLevel tob1{5, "BTC/USDT", MarketOrder{99.5, 1.0}, MarketOrder{100.5, 1.0}};
    const OrderBookLevel tob2{3, "BTC/USDT", MarketOrder{99.5, 1.0}, MarketOrder{100.5, 1.0}}; // stale

    EXPECT_CALL(*mockPtr, update(testing::_)).Times(1);
    EXPECT_CALL(*mockPtr, getMetric()).WillOnce(testing::Return(1.0));

    subject->processData(tob1);
    subject->processData(tob2);
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

TEST_F(SignalEngineTests, shouldChangeBuySellDecisionAfterDifferentTopOfOrderBooks) {
    GTEST_SKIP() << "Not implemented";
}

// TODO make these tests test that we make calls to the OrderExecution.