#pragma once
#include <deque>

#include "Metric.hpp"

namespace dataProcessing::metrics {
    class BidAskVolumeRatio : public Metric {
        uint16_t lookback;
        std::deque<std::pair<double, double>> historicData;
        double ratio{};
        long double bidVol{};
        long double askVol{};

        void updateRatio();

        void update(double newBidQuantity, double newAskQuantity);
    public:
        explicit BidAskVolumeRatio(uint16_t lookback);
        void update(const TopOfBook& topOfBook) override;

        [[nodiscard]] double getMetric() const override;
    };
}