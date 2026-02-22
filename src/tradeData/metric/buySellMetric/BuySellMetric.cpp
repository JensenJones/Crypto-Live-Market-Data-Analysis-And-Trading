#include "tradeData/metric/buySellMetric/BuySellMetric.hpp"

namespace tradeData::metrics {
    BuySellMetric::BuySellMetric(const long lookback_, const BuySellDecision tradingIndicator) : Metric(lookback_, tradingIndicator) {}

    std::optional<Order::BuySell> BuySellMetric::getBuySellIndication() const {
        return tradingIndicator(metricValue);
    }
} // metrics