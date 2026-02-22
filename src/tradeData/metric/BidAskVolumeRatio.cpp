#include "../../../include/tradeData/metric/BidAskVolumeRatio.hpp"

#include <iostream>

namespace tradeData::metrics {
    void BidAskVolumeRatio::updateRatio() {
        ratio = bidVol / std::max(askVol, 1e-6L);
    }

    void BidAskVolumeRatio::update(double newBidQuantity, double newAskQuantity) {
        historicData.emplace_back(newBidQuantity, newAskQuantity);
        bidVol += newBidQuantity;
        askVol += newAskQuantity;

        if (historicData.size() > lookback) {
            auto [removingBidVol, removingAskVol] = historicData.front();
            historicData.pop_front();

            bidVol -= removingBidVol;
            askVol -= removingAskVol;
        }

        updateRatio();
    }

    BidAskVolumeRatio::BidAskVolumeRatio(const uint16_t lookback) : lookback{ lookback } {}

    void BidAskVolumeRatio::update(const OrderBookLevel &orderBookLevel) {
        update(orderBookLevel.getBestBid().getQuantity(), orderBookLevel.getBestAsk().getQuantity());
    }

    double BidAskVolumeRatio::getMetric() const {
        return ratio;
    }
}
