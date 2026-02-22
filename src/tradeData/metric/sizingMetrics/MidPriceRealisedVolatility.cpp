#include "tradeData/metric/sizingMetric/MidPriceRealisedVolatility.hpp"

#include <cmath>

#include "tradeData/metric/buySellMetric/BidAskVolumeRatio.hpp"

namespace tradeData::metrics {
    MidPriceRealisedVolatility::MidPriceRealisedVolatility(uint16_t lookback_,
                                                           ::metrics::SizingDecision tradingIndicator_) :
    SizingMetric(lookback_, tradingIndicator_){}

    void MidPriceRealisedVolatility::updateMetric() {
        metricValue = std::sqrt(historicLogReturnsSum / static_cast<long double>(historicDataTimeSpacingSum));
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

        updateMetric();
    }

    void MidPriceRealisedVolatility::update(const OrderBookLevel &orderBookLevel) {
        const auto bestBid = orderBookLevel.getBestBid().getPrice();
        const auto bestAsk = orderBookLevel.getBestAsk().getPrice();
        const auto newMidPrice = (bestBid + bestAsk) / 2.0;
        const auto newUpdateId = orderBookLevel.getUpdateId();

        if (!initialised) {
            initialised = true;
        } else {
            update(newMidPrice, newUpdateId);
        }

        lastMidPrice = newMidPrice;
        lastUpdateId = newUpdateId;
    }

    double MidPriceRealisedVolatility::getMetric() const {
        return metricValue;
    }
}
