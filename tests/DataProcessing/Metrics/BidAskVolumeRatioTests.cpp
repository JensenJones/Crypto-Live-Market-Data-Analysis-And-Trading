#include "../../../include/DataProcessing/Metrics/BidAskVolumeRatio.hpp"

#include <gtest/gtest.h>

using bavr = dataProcessing::metrics::BidAskVolumeRatio;

class BidAskVolumeRatioTests : public testing::Test {
protected:
    constexpr static uint16_t LOOKBACK{5};
    bavr subject{LOOKBACK};

    void SetUp() override {
    }
};

TEST_F(BidAskVolumeRatioTests, shouldReturnZeroForInitialRatio) {
    const auto ratio = subject.getMetric();

    ASSERT_EQ(ratio, 0.0);
}

TEST_F(BidAskVolumeRatioTests, shouldReturnARatioOnceUpdatedLookbackTimes) {
    double totalBidQuanty{};
    double totalAskQuantity{};

    for (int i = 0; i < LOOKBACK; ++i) {
        double newBidQuantity = 1;
        double newAskQuantity = 2 + i;

        subject.update(TopOfBook(i, "BTC/USDT",
            Order(10, newBidQuantity),
            Order(11, newAskQuantity)));

        totalBidQuanty += newBidQuantity;
        totalAskQuantity += newAskQuantity;

        const auto ratio = subject.getMetric();
        ASSERT_NEAR(ratio, totalBidQuanty / totalAskQuantity, 1e-6);
    }
}

TEST_F(BidAskVolumeRatioTests, shouldFIFOEvictAndUpdateRatioAcordingly) {
    double windowBidQuantity{};
    std::vector<double> bidQuantitiesAdded{};
    double windowAskQuantity{};
    std::vector<double> askQuantitiesAdded{};

    for (int i = 0; i < LOOKBACK; ++i) {
        constexpr double newBidQuantity = 1;
        const double newAskQuantity = 2 + i;

        subject.update(TopOfBook(i, "BTC/USDT",
            Order(10, newBidQuantity),
            Order(11, newAskQuantity)));

        windowBidQuantity += newBidQuantity;
        bidQuantitiesAdded.push_back(newBidQuantity);
        windowAskQuantity += newAskQuantity;
        askQuantitiesAdded.push_back(newAskQuantity);
    }

    for (int i = 0; i < bidQuantitiesAdded.size(); ++i) {
        constexpr double newBidQuantity = 1;
        constexpr double newAskQuantity = 1;
        subject.update(TopOfBook(i, "BTC/USDT",
        Order(10, newBidQuantity),
        Order(11, newAskQuantity)));

        windowBidQuantity += newBidQuantity;
        windowAskQuantity += newAskQuantity;
        windowBidQuantity -= bidQuantitiesAdded[i];
        windowAskQuantity -= askQuantitiesAdded[i];

        const auto ratio = subject.getMetric();
        ASSERT_NEAR(ratio, windowBidQuantity / windowAskQuantity, 1e-6);
    }
}


TEST_F(BidAskVolumeRatioTests, shouldClampAskVolumeWhenZeroToAvoidDivisionByZero) {
    constexpr double bidQty = 10.0;
    constexpr double askQty = 0.0;

    TopOfBook tob{0, "BTC/USDT",
                  Order(100.0, bidQty),
                  Order(101.0, askQty)};

    subject.update(tob);

    // Implementation uses: ratio = bidVol / max(askVol, 1e-6)
    constexpr double expected = bidQty / 0.000001;
    const auto ratio = subject.getMetric();

    ASSERT_NEAR(ratio, expected, 1e-6);
}

TEST_F(BidAskVolumeRatioTests, shouldClampAskVolumeWhenVerySmall) {
    constexpr double bidQty = 5.0;
    constexpr double tinyAsk = 0.5e-6; // less than 1e-6 threshold

    const TopOfBook tob{1, "BTC/USDT",
                  Order(100.0, bidQty),
                  Order(101.0, tinyAsk)};

    subject.update(tob);

    constexpr double expected = bidQty / 0.000001;
    const auto ratio = subject.getMetric();

    ASSERT_NEAR(ratio, expected, 1e-6);
}

TEST(BidAskVolumeRatioLookbackOneTests, shouldUseOnlyLastSampleWhenLookbackIsOne) {
    constexpr uint16_t lookback = 1;
    bavr subject{lookback};

    auto makeTopOfBook = [](const uint64_t id, const double bidQty, const double askQty) {
        return TopOfBook{id, "BTC/USDT",
                         Order(100.0, bidQty),
                         Order(101.0, askQty)};
    };

    subject.update(makeTopOfBook(0, 2.0, 4.0));
    ASSERT_NEAR(subject.getMetric(), 2.0 / 4.0, 1e-12);

    subject.update(makeTopOfBook(1, 5.0, 10.0));
    ASSERT_NEAR(subject.getMetric(), 5.0 / 10.0, 1e-12);

    subject.update(makeTopOfBook(2, 3.0, 6.0));
    ASSERT_NEAR(subject.getMetric(), 3.0 / 6.0, 1e-12);
}

TEST_F(BidAskVolumeRatioTests, shouldMatchManualSlidingWindowForArbitrarySequence) {
    struct Sample {
        double bidQty;
        double askQty;
    };

    std::vector<Sample> samples{
        {1.0,  3.0},
        {2.0,  5.0},
        {4.0,  7.0},
        {1.5, 2.5},
        {3.0,  1.0},
        {2.0, 10.0},
        {6.0,  4.0},
    };

    std::deque<Sample> window;
    double windowBidSum = 0.0;
    double windowAskSum = 0.0;

    uint64_t id = 0;
    for (const auto& s : samples) {
        TopOfBook tob{id++, "BTC/USDT",
                      Order(100.0, s.bidQty),
                      Order(101.0, s.askQty)};

        subject.update(tob);

        window.push_back(s);
        windowBidSum += s.bidQty;
        windowAskSum += s.askQty;

        if (window.size() > LOOKBACK) {
            windowBidSum -= window.front().bidQty;
            windowAskSum -= window.front().askQty;
            window.pop_front();
        }

        if (!window.empty()) {
            const double denom = std::max(windowAskSum, 0.000001);
            const double expected = windowBidSum / denom;
            ASSERT_NEAR(subject.getMetric(), expected, 1e-6);
        }
    }
}