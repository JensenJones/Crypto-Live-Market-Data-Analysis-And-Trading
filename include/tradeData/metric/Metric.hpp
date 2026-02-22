#pragma once
#include "messageHandling/OrderBookLevel.hpp"

namespace tradeData::metrics {

    template<typename T>
    concept TradingIndicatorStruct = requires(T function, double metricValue) {
            { function(metricValue) };
    };
    template <typename TradingIndicatorStruct>
    class Metric {
    protected:
        long lookback;
        TradingIndicatorStruct tradingIndicator;
        double metricValue;

    public:
        virtual ~Metric() = default;
        Metric(long lookback_, TradingIndicatorStruct tradingIndicator_);

        virtual void update(const OrderBookLevel& orderBookLevel) = 0;
        [[nodiscard]] virtual double getMetric() const = 0;

        void setDecisionFn(const TradingIndicatorStruct& newTradingIndicator);
    };

    template<typename TradingIndicatorStruct>
    Metric<TradingIndicatorStruct>::Metric(const long lookback_, TradingIndicatorStruct tradingIndicator_) :
        lookback(lookback_), tradingIndicator(tradingIndicator_), metricValue{} {}

    template<typename TradingIndicatorStruct>
    void Metric<TradingIndicatorStruct>::setDecisionFn(const TradingIndicatorStruct &newTradingIndicator) {
        tradingIndicator = newTradingIndicator;
    }
}
