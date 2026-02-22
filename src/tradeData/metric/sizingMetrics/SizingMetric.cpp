#include "tradeData/metric/sizingMetric/SizingMetric.hpp"

namespace metrics {
    SizingMetric::SizingMetric(const long lookback_, const SizingDecision tradingIndicator) : Metric(lookback_, tradingIndicator) {}

    double SizingMetric::getBuySellIndication() const {
        return tradingIndicator(metricValue);
    }
} // metrics