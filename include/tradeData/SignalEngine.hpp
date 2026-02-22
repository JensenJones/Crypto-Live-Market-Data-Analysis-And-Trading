#pragma once

#include <expected>
#include <string>
#include <utility>
#include <optional>
#include <unordered_map>

#include "execution/OrderExecution.hpp"
#include "tradeData/metric/Metric.hpp"
#include "tradeData/metric/MetricNames.hpp"
#include "order/BuySell.hpp"

namespace tradeData {

    class SignalEngine {
        using metricUp = std::unique_ptr<metrics::Metric>;
        using doublePair = std::pair<double, double>;

        const std::string symbol;
        execution::OrderExecution& orderExecutor;

        uint64_t latestUpdateId{};
        std::unique_ptr<OrderBookLevel> lastOrderBookLevelUp = nullptr;

        std::unordered_map<MetricName, metricUp> buySellMetricCalculators;
        std::unordered_map<MetricName, doublePair> buySellMetricLimits;

        std::unordered_map<MetricName, metricUp> sizingMetricCalculators;

        uint32_t metricCount{};

        void updateMetrics(const OrderBookLevel & orderBookLevel) const;

        std::optional<Order::BuySell> orderConditionsMet();

    public:
        SignalEngine(std::string symbol, execution::OrderExecution& orderExecutor_);

        void processData(const OrderBookLevel& orderBookLevel);

        void addBuySellMetric(MetricName metricName, metricUp metric, const doublePair &limits);
        void updateBuySellMetricLimit(MetricName metricName, doublePair limits);

        void addSizingMetric(MetricName metricName, metricUp metric);

        std::expected<bool, std::string> removeMetric(MetricName metricName);

        OrderBookLevel getLastProcessedData() const;
    };
}
