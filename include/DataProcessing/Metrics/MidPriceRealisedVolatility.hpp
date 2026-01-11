#pragma once
#include <cstdint>
#include <deque>

#include "Metric.hpp"
#include "MessageHandling/TopOfBook.hpp"

namespace dataProcessing::metrics {
    class MidPriceRealisedVolatility : Metric{
        uint16_t lookback;
        std::deque<double> historicLogReturnsSquared;
        std::deque<uint16_t> historicDataTimeSpacing;

        uint64_t lastUpdateId{};
        double lastMidPrice{};

        long double historicLogReturnsSum{};
        uint64_t historicDataTimeSpacingSum{};
        double volMetric{};
        bool initialised{};

        void updateVol();
        void update(double newMidPrice, uint16_t newUpdateId);

    public:
        explicit MidPriceRealisedVolatility(uint16_t lookback);
        void update(const TopOfBook& topOfBook) override;

        [[nodiscard]] double getMetric() const override;
    };
}

