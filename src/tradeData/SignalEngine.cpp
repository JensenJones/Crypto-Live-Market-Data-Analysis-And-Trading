#include "tradeData/SignalEngine.hpp"

#include <ranges>
#include <utility>

namespace tradeData {
    SignalEngine::SignalEngine(std::string symbol_, execution::OrderExecution &orderExecutor_) :
        symbol(std::move(symbol_)), orderExecutor(orderExecutor_) {}

    void SignalEngine::updateMetrics(const OrderBookLevel &orderBookLevel) const {
        for (const auto &metric: buySellMetricCalculators | std::views::values) {
            metric->update(orderBookLevel);
        }

        for (const auto& metric : sizingMetricCalculators | std::views::values) {
            metric->update(orderBookLevel);
        }
    }

    std::optional<Order::BuySell> SignalEngine::orderConditionsMet() {
        uint32_t sellWeight{};
        uint32_t buyWeight{};

        for (const auto& [metricName, metricUp] : buySellMetricCalculators) {
            const auto &[sellLimit, buyLimit] = buySellMetricLimits[metricName];
            if (const double metric = metricUp->getMetric(); metric < sellLimit) {
                ++sellWeight;
            } else if (metric > buyLimit) {
                ++buyWeight;
            } else {
                return std::nullopt; // only want to order if all indicators indicate one way
            }
        }

        if (buyWeight == metricCount) {
            return std::optional(Order::BuySell::BUY);
        }

        if (sellWeight == metricCount) {
            return std::optional(Order::BuySell::SELL);
        }

        return std::nullopt;
    }

    void SignalEngine::processData(const OrderBookLevel& orderBookLevel) {
        if (const uint64_t newLatestUpdateId = orderBookLevel.getUpdateId();
            newLatestUpdateId > latestUpdateId) {
            lastOrderBookLevelUp = std::make_unique<OrderBookLevel>(orderBookLevel);

            updateMetrics(orderBookLevel);
            latestUpdateId = newLatestUpdateId;

            if (const auto buySellOpt = orderConditionsMet()) {
                const Order::BuySell buySell = buySellOpt.value();
                const auto price = buySell == Order::BuySell::BUY ?
                                       orderBookLevel.getBestBid().getPrice() : orderBookLevel.getBestAsk().getPrice();
                orderExecutor.submitOrder(buySell, 0.001, price); // Order quantity 1 at bid/ask market price
            }
        }
    }

    void SignalEngine::addBuySellMetric(const MetricName metricName, metricUp metric, const doublePair &limits) {
        if (buySellMetricCalculators.contains(metricName)) {
            throw std::runtime_error("Metric Already Exists");
        }

        buySellMetricCalculators[metricName] = std::move(metric);
        buySellMetricLimits[metricName] = limits;
        ++metricCount;
    }

    std::expected<bool, std::string> SignalEngine::removeMetric(const MetricName metricName) {
        if (buySellMetricCalculators.contains(metricName)) {
            buySellMetricCalculators.erase(metricName);
            buySellMetricLimits.erase(metricName);
        } else if (sizingMetricCalculators.contains(metricName)) {
            sizingMetricCalculators.erase(metricName);
        } else {
            return std::unexpected("Metric never added");
        }

        return true;
    }

    void SignalEngine::updateBuySellMetricLimit(const MetricName metricName, doublePair limits) {
        buySellMetricLimits[metricName] = std::move(limits);
    }

    OrderBookLevel SignalEngine::getLastProcessedData() const { return *lastOrderBookLevelUp; }
}
