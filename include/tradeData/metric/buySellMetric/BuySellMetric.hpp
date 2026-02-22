#pragma once
#include "order/BuySell.hpp"
#include "tradeData/metric/Metric.hpp"

struct BuySellDecision {
    double buyThreshold;
    double sellThreshold;

    std::optional<Order::BuySell> operator()(const double metricValue) const {
        if (metricValue >= buyThreshold) return std::optional{Order::BuySell::BUY};
        if (metricValue <= sellThreshold) return std::optional{Order::BuySell::SELL};
        return std::nullopt;
    }
};


namespace metrics {
    class BuySellMetric : tradeData::metrics::Metric<BuySellDecision> {
    public:
        BuySellMetric(long lookback_, BuySellDecision tradingIndicator);
        [[nodiscard]] virtual std::optional<Order::BuySell> getBuySellIndication() const;
    };
}
