#include "DataProcessing/Metrics/MidPriceRealisedVolatility.hpp"

#include <cmath>

#include "../../../include/DataProcessing/Metrics/BidAskVolumeRatio.hpp"

#include <iostream>

namespace dataProcessing::metrics {
    MidPriceRealisedVolatility::MidPriceRealisedVolatility(const uint16_t lookback) : lookback( lookback ) {}

    void MidPriceRealisedVolatility::updateVol() {
        volMetric = std::sqrt(historicLogReturnsSum / historicDataTimeSpacingSum);
    }

    void MidPriceRealisedVolatility::update(const double newMidPrice, const uint16_t newUpdateId) {
        const auto updateSpacing = newUpdateId - lastUpdateId;
        const double logReturns = std::log(newMidPrice / lastMidPrice);
        const double logReturnsSquared = logReturns * logReturns;

        historicLogReturnsSquared.emplace_back(logReturnsSquared);
        historicDataTimeSpacing.emplace_back(updateSpacing);

        historicLogReturnsSum += logReturnsSquared;
        historicDataTimeSpacingSum += updateSpacing;

        if (historicLogReturnsSquared.size() > lookback) {
            const double rmLogReturn = historicLogReturnsSquared.front();
            const uint16_t rmTimeSpacing = historicDataTimeSpacing.front();

            historicLogReturnsSquared.pop_front();
            historicDataTimeSpacing.pop_front();

            historicLogReturnsSum -= rmLogReturn;
            historicDataTimeSpacingSum -= rmTimeSpacing;
        }

        updateVol();
    }

    void MidPriceRealisedVolatility::update(const TopOfBook &topOfBook) {
        const auto bestBid = topOfBook.getBestBid().getPrice();
        const auto bestAsk = topOfBook.getBestAsk().getPrice();
        const auto newMidPrice = (bestBid + bestAsk) / 2.0;
        const auto newUpdateId = topOfBook.getUpdateId();

        if (!initialised) {
            initialised = true;
        } else {
            update(newMidPrice, newUpdateId);
        }

        lastMidPrice = newMidPrice;
        lastUpdateId = newUpdateId;
    }

    double MidPriceRealisedVolatility::getMetric() const {
        return volMetric;
    }
}
