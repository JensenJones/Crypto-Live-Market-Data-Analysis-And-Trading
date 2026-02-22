#include "tradeData/metric/sizingMetric/SizingMetric.hpp"

namespace tradeData::metrics {
    SizingMetric::SizingMetric(const long lookback_, const SizingDecision tradingIndicator) : Metric(lookback_, tradingIndicator) {}

    double SizingMetric::getSizingIndication() const {
        return tradingIndicator(metricValue);
    }
} // metrics