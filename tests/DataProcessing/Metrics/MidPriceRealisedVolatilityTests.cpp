#include <gtest/gtest.h>

#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

#include "DataProcessing/Metrics/MidPriceRealisedVolatility.hpp"
#include "MessageHandling/TopOfBook.hpp"
#include "MessageHandling/Order.hpp"   // adjust if your Order header path differs

namespace dataProcessing::metrics {

namespace {
    TopOfBook MakeTob(const uint64_t updateId,
                      const double bidPrice,
                      const double askPrice,
                      const double bidQty = 1.0,
                      const double askQty = 1.0,
                      const std::string& symbol = "BTC/USDT") {
        return {updateId, symbol, Order(bidPrice, bidQty), Order(askPrice, askQty)};
    }

    double MidFrom(const double bid, const double ask) {
        return (bid + ask) / 2.0;
    }

    double ExpectedVolFromWindow(const std::vector<double>& mids,
                                 const std::vector<uint64_t>& updateIds) {
        long double sumR2 = 0.0L;
        long double sumDt = 0.0L;

        for (size_t i = 1; i < mids.size(); ++i) {
            const double r = std::log(mids[i] / mids[i - 1]);
            const double r2 = r * r;
            const uint64_t dt = updateIds[i] - updateIds[i - 1];

            sumR2 += r2;
            sumDt += static_cast<long double>(dt);
        }

        return std::sqrt(static_cast<double>(sumR2 / sumDt));
    }
}

class MidPriceRealisedVolatilityTests : public ::testing::Test {};

TEST_F(MidPriceRealisedVolatilityTests, warmupFirstUpdateSetsStateAndMetricRemainsZero) {
    MidPriceRealisedVolatility subject{5};

    const auto tob0 = MakeTob(100, 10.0, 12.0);
    subject.update(tob0);

    EXPECT_NEAR(subject.getMetric(), 0.0, 0.0);
}

TEST_F(MidPriceRealisedVolatilityTests, computesVolUsingMidPriceAndUpdateIdDeltaForSingleReturn) {
    MidPriceRealisedVolatility subject{5};

    const auto tob0 = MakeTob(100, 10.0, 12.0);
    const auto tob1 = MakeTob(110, 11.0, 13.0);

    subject.update(tob0);
    subject.update(tob1);

    const double mid0 = MidFrom(10.0, 12.0);
    const double mid1 = MidFrom(11.0, 13.0);

    const double r = std::log(mid1 / mid0);
    const double expected = std::sqrt((r * r) / 10.0);

    EXPECT_NEAR(subject.getMetric(), expected, 1e-12);
}

TEST_F(MidPriceRealisedVolatilityTests, accumulatesOverMultipleUpdatesWithNonUniformUpdateIdDeltas) {
    MidPriceRealisedVolatility subject{10};

    // Build a small path with varying dt
    // updateIds: 100 -> 103 -> 110 -> 111 (dt: 3, 7, 1)
    // mids derived from bid/ask
    const auto tob0 = MakeTob(100, 100.0, 102.0); // mid=101
    const auto tob1 = MakeTob(103, 101.0, 103.0); // mid=102
    const auto tob2 = MakeTob(110, 103.0, 105.0); // mid=104
    const auto tob3 = MakeTob(111, 104.0, 106.0); // mid=105

    subject.update(tob0); // warmup
    subject.update(tob1);
    subject.update(tob2);
    subject.update(tob3);

    const std::vector<double> mids{
        MidFrom(100.0, 102.0),
        MidFrom(101.0, 103.0),
        MidFrom(103.0, 105.0),
        MidFrom(104.0, 106.0),
    };
    const std::vector<uint64_t> ids{100, 103, 110, 111};

    const double expected = ExpectedVolFromWindow(mids, ids);
    EXPECT_NEAR(subject.getMetric(), expected, 1e-12);
}

TEST_F(MidPriceRealisedVolatilityTests, enforcesLookbackByDroppingOldestReturnContribution) {
    // lookback counts RETURNS (not raw samples) in your implementation pattern.
    // So lookback=2 means: keep the last 2 squared returns (and their dt).
    MidPriceRealisedVolatility subject{2};

    // 4 updates => 3 returns; oldest return should be evicted.
    // ids: 100 -> 101 -> 103 -> 106  (dt: 1, 2, 3)
    // mids: 101 -> 102 -> 104 -> 105 (simple)
    const auto tob0 = MakeTob(100, 100.0, 102.0); // mid=101
    const auto tob1 = MakeTob(101, 101.0, 103.0); // mid=102
    const auto tob2 = MakeTob(103, 103.0, 105.0); // mid=104
    const auto tob3 = MakeTob(106, 104.0, 106.0); // mid=105

    subject.update(tob0); // warmup
    subject.update(tob1); // return #1
    subject.update(tob2); // return #2
    subject.update(tob3); // return #3 -> should evict return #1

    // Expected window for last 2 returns only: (tob1->tob2) and (tob2->tob3)
    const std::vector<double> midsTail{
        MidFrom(101.0, 103.0), // mid at id 101
        MidFrom(103.0, 105.0), // mid at id 103
        MidFrom(104.0, 106.0), // mid at id 106
    };
    const std::vector<uint64_t> idsTail{101, 103, 106};

    const double expected = ExpectedVolFromWindow(midsTail, idsTail);
    EXPECT_NEAR(subject.getMetric(), expected, 1e-12);
}

TEST_F(MidPriceRealisedVolatilityTests, midPriceIsComputedFromBestBidAndBestAskNotQuantities) {
    MidPriceRealisedVolatility subject{5};

    // Same prices, wildly different quantities: mid should be identical, so return should be 0.
    const auto tob0 = MakeTob(100, 10.0, 20.0, /*bidQty*/ 1.0, /*askQty*/ 9999.0);
    const auto tob1 = MakeTob(101, 10.0, 20.0, /*bidQty*/ 9999.0, /*askQty*/ 1.0);

    subject.update(tob0); // warmup
    subject.update(tob1);

    EXPECT_NEAR(subject.getMetric(), 0.0, 1e-12);
}

} // namespace dataProcessing::metrics
