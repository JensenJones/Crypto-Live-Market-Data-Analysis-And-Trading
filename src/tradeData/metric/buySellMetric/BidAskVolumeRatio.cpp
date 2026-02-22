#include "../../../../include/tradeData/metric/buySellMetric/BidAskVolumeRatio.hpp"

#include <iostream>

namespace tradeData::metrics {
    void BidAskVolumeRatio::updateMetricValue() {
        metricValue = bidVol / std::max(askVol, 1e-6L);
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

        updateMetricValue();
    }

    BidAskVolumeRatio::BidAskVolumeRatio(const uint16_t lookback_, const ::metrics::BuySellDecision tradingIndicator) :
        BuySellMetric(lookback_, tradingIndicator) {}

    void BidAskVolumeRatio::update(const OrderBookLevel &orderBookLevel) {
        update(orderBookLevel.getBestBid().getQuantity(), orderBookLevel.getBestAsk().getQuantity());
    }

    double BidAskVolumeRatio::getMetric() const {
        return metricValue;
    }
}
