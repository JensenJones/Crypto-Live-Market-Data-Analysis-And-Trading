#include "tradeData/SignalEngine.hpp"

#include <ranges>
#include <utility>

namespace tradeData {
    SignalEngine::SignalEngine(std::string symbol_, execution::OrderExecution &orderExecutor_) :
        symbol(std::move(symbol_)), orderExecutor(orderExecutor_) {}

    void SignalEngine::updateMetrics(const OrderBookLevel &orderBookLevel) const {
        for (const auto& metric: buySellMetricCalculators | std::views::values) {
            metric->update(orderBookLevel);
        }

        for (const auto& metric : sizingMetricCalculators | std::views::values) {
            metric->update(orderBookLevel);
        }
    }

    std::optional<Order::BuySell> SignalEngine::orderConditionsMet() {
        uint32_t sellWeight{};
        uint32_t buyWeight{};

        for (const auto &metricUp: buySellMetricCalculators | std::views::values) {
            if (auto buySell = metricUp->getBuySellIndication()) {
                if (buySell == Order::BuySell::BUY) {
                    ++buyWeight;
                } else {
                    ++sellWeight;
                }
            } else {
                return std::nullopt;
            }
        }

        if (buyWeight == buySellMetricCount) {
            return std::optional{Order::BuySell::BUY};
        }

        if (sellWeight == buySellMetricCount) {
            return std::optional{Order::BuySell::SELL};
        }

        return std::nullopt;
    }

    double SignalEngine::calculateOrderSize() const {
        long double sizeIndicatorSum{};

        for (const auto& metricUp : sizingMetricCalculators | std::views::values) {
            sizeIndicatorSum += metricUp->getSizingIndication();
        }

        return sizeIndicatorSum / sizingMetricCount;
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
                orderExecutor.submitOrder(buySell, calculateOrderSize(), price); // Order quantity 1 at bid/ask market price
            }
        }
    }

    void SignalEngine::addBuySellMetric(const MetricName metricName, buySellMetricUp metric) {
        if (buySellMetricCalculators.contains(metricName)) {
            throw std::runtime_error("Metric Already Exists");
        }

        buySellMetricCalculators[metricName] = std::move(metric);
        ++buySellMetricCount;
    }

    void SignalEngine::updateBuySellMetricDecision(const MetricName metricName, const metrics::BuySellDecision buySellDecision) {
        if (!buySellMetricCalculators.contains(metricName)) {
            throw std::runtime_error("Cannot update a metric that doesnt exist");
        }

        buySellMetricCalculators[metricName]->setDecisionFn(buySellDecision);
    }

    void SignalEngine::addSizingMetric(const MetricName metricName, sizingMetricUp metric) {
        if (sizingMetricCalculators.contains(metricName)) {
            throw std::runtime_error("Metric Already Exists");
        }

        sizingMetricCalculators[metricName] = std::move(metric);
        ++sizingMetricCount;
    }

    void SignalEngine::updateSizingMetricDecision(const MetricName metricName, const metrics::SizingDecision sizingDecision) {
        if (!sizingMetricCalculators.contains(metricName)) {
            throw std::runtime_error("Cannot update a metric that doesnt exist");
        }

        sizingMetricCalculators[metricName]->setDecisionFn(sizingDecision);
    }

    std::expected<bool, std::string> SignalEngine::removeMetric(const MetricName metricName) {
        if (buySellMetricCalculators.contains(metricName)) {
            buySellMetricCalculators.erase(metricName);
            --buySellMetricCount;
        } else if (sizingMetricCalculators.contains(metricName)) {
            sizingMetricCalculators.erase(metricName);
            --sizingMetricCount;
        } else {
            return std::unexpected("Metric never added");
        }

        return true;
    }

    OrderBookLevel SignalEngine::getLastProcessedData() const { return *lastOrderBookLevelUp; }
}
