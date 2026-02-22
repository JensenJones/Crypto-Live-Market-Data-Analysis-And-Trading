#pragma once
#include <deque>

#include "BuySellMetric.hpp"
#include "../Metric.hpp"

namespace tradeData::metrics {
    class BidAskVolumeRatio final : public BuySellMetric {
        std::deque<std::pair<double, double>> historicData;
        double metricValue{};
        long double bidVol{};
        long double askVol{};

        void updateMetricValue();

        void update(double newBidQuantity, double newAskQuantity);
    public:
        explicit BidAskVolumeRatio(uint16_t lookback_, BuySellDecision tradingIndicator);
        void update(const OrderBookLevel& orderBookLevel) override;

        [[nodiscard]] double getMetric() const override;
    };
}