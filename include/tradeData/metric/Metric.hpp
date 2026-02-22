#pragma once
#include "messageHandling/OrderBookLevel.hpp"


namespace tradeData::metrics {
    class Metric {
    public:
        virtual ~Metric() = default;
        virtual void update(const OrderBookLevel& orderBookLevel) = 0;
        [[nodiscard]] virtual double getMetric() const = 0;
    };
}
