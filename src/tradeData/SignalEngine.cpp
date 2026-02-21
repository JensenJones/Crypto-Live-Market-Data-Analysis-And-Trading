#include "tradeData/SignalEngine.hpp"

#include <ranges>
#include <utility>

namespace tradeData {
    SignalEngine::SignalEngine(std::string symbol_, execution::OrderExecution &orderExecutor_) :
        symbol(std::move(symbol_)), orderExecutor(orderExecutor_) {}

    void SignalEngine::updateMetrics(const OrderBookLevel &orderBookLevel) const {
        for (const auto &metric: metricCalculators | std::views::values) {
            metric->update(orderBookLevel);
        }
    }

    std::optional<Order::BuySell> SignalEngine::orderConditionsMet() {
        uint32_t sellWeight{};
        uint32_t buyWeight{};

        for (const auto& [metricName, metricUp] : metricCalculators) {
            const auto &[sellLimit, buyLimit] = metricLimits[metricName];
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
            updateMetrics(orderBookLevel);
            latestUpdateId = newLatestUpdateId;

            if (const auto buySellOpt = orderConditionsMet()) {
                const Order::BuySell buySell = buySellOpt.value();
                const auto price = buySell == Order::BuySell::BUY ?
                                       orderBookLevel.getBestBid().getPrice() : orderBookLevel.getBestAsk().getPrice();
                orderExecutor.submitOrder(buySell, 1, price); // Order quantity 1 at bid/ask market price
            }
        }
    }

    void SignalEngine::addMetric(const MetricName metricName, metricUp metric, const limitPair &limits) {
        if (metricCalculators.contains(metricName)) {
            throw std::runtime_error("Metric Already Exists");
        }

        metricCalculators[metricName] = std::move(metric);
        metricLimits[metricName] = limits;
        ++metricCount;
    }

    std::expected<bool, std::string> SignalEngine::removeMetric(const MetricName metricName) {
        if (!metricCalculators.contains(metricName)) {
            return std::unexpected("Metric never added");
        }

        metricCalculators.erase(metricName);

        if (metricLimits.contains(metricName)) {
            metricLimits.erase(metricName);
        }

        return true;
    }

    void SignalEngine::updateLimit(const MetricName metricName, limitPair limits) {
        metricLimits[metricName] = std::move(limits);
    }
}