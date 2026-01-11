#pragma once

#include "../../MessageHandling/TopOfBook.hpp"

namespace dataProcessing::metrics {
    class Metric {
    public:
        virtual ~Metric() = default;
        virtual void update(const TopOfBook& topOfBook) = 0;
        [[nodiscard]] virtual double getMetric() const = 0;
    };
}
