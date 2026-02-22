#pragma once
#include "order/BuySell.hpp"
#include "tradeData/metric/Metric.hpp"


namespace tradeData::metrics {
    struct BuySellDecision {
        double buyThreshold;
        double sellThreshold;

        std::optional<Order::BuySell> operator()(const double metricValue) const {
            if (metricValue >= buyThreshold) return std::optional{Order::BuySell::BUY};
            if (metricValue <= sellThreshold) return std::optional{Order::BuySell::SELL};
            return std::nullopt;
        }
    };

    class BuySellMetric : public Metric<BuySellDecision> {
    public:
        BuySellMetric(long lookback_, BuySellDecision tradingIndicator);
        [[nodiscard]] virtual std::optional<Order::BuySell> getBuySellIndication() const;
    };
}
