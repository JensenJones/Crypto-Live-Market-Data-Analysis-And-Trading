#pragma once
#include "tradeData/metric/Metric.hpp"

namespace metrics {
    class SizingMetric : tradeData::metrics::Metric {
    public:
        [[nodiscard]] virtual double getBuySellIndication() = 0;
    };
}
