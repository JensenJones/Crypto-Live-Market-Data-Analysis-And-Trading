#pragma once
#include "tradeData/metric/Metric.hpp"


namespace metrics {
    struct SizingDecision {
        double targetCapital;

        double operator()(const double metricValue) const {
            return targetCapital / (metricValue * 1000);
        }
    };

    class SizingMetric : public tradeData::metrics::Metric<SizingDecision> {
    public:
        SizingMetric(long lookback_, SizingDecision tradingIndicator);
        [[nodiscard]] virtual double getBuySellIndication() const;
    };
}
