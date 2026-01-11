#include "../../../include/DataProcessing/Metrics/BidAskVolumeRatio.hpp"

#include <iostream>

namespace dataProcessing::metrics {
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

    void BidAskVolumeRatio::update(const TopOfBook &topOfBook) {
        update(topOfBook.getBestBid().getQuantity(), topOfBook.getBestAsk().getQuantity());
    }

    double BidAskVolumeRatio::getMetric() const {
        return ratio;
    }
}
