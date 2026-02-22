#pragma once
#include "order/BuySell.hpp"
#include "tradeData/metric/Metric.hpp"

namespace metrics {
    class BuySellMetric : tradeData::metrics::Metric {
    public:
        [[nodiscard]] virtual std::optional<Order::BuySell> getBuySellIndication() = 0;
    };
}
