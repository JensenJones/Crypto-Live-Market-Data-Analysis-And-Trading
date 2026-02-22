#pragma once
#include "tradeData/metric/Metric.hpp"

struct TradeSizing {
    double targetCapital;

    double operator()(const double metricValue) const {
        return targetCapital / (metricValue * 1000);
    }
};

namespace metrics {
    class SizingMetric : tradeData::metrics::Metric<TradeSizing> {
    public:
        SizingMetric(long lookback_, TradeSizing tradingIndicator);
        [[nodiscard]] virtual double getBuySellIndication() const;
    };
}
