#pragma once
#include <deque>

#include "Metric.hpp"

namespace tradeData::metrics {
    class BidAskVolumeRatio final : public Metric {
        uint16_t lookback;
        std::deque<std::pair<double, double>> historicData;
        double ratio{};
        long double bidVol{};
        long double askVol{};

        void updateRatio();

        void update(double newBidQuantity, double newAskQuantity);
    public:
        explicit BidAskVolumeRatio(uint16_t lookback);
        void update(const OrderBookLevel& orderBookLevel) override;

        [[nodiscard]] double getMetric() const override;
    };
}