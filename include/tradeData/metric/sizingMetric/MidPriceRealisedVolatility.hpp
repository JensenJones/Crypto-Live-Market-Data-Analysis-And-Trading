#pragma once
#include <deque>

#include "SizingMetric.hpp"
#include "messageHandling/OrderBookLevel.hpp"

namespace tradeData::metrics {
    class MidPriceRealisedVolatility final : public SizingMetric {
        std::deque<double> historicLogReturnsSquared;
        std::deque<uint16_t> historicDataTimeSpacing;

        uint64_t lastUpdateId{};
        double lastMidPrice{};

        long double historicLogReturnsSum{};
        uint64_t historicDataTimeSpacingSum{};
        bool initialised{};

        void updateMetric();
        void update(double newMidPrice, uint16_t newUpdateId);

    public:
        explicit MidPriceRealisedVolatility(uint16_t lookback_, SizingDecision tradingIndicator_);
        void update(const OrderBookLevel& orderBookLevel) override;

        [[nodiscard]] double getMetric() const override;
    };
}

