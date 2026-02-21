#pragma once

#include <expected>
#include <string>
#include <utility>
#include <optional>
#include <unordered_map>

#include "execution/OrderExecution.hpp"
#include "metrics/Metric.hpp"
#include "metrics/MetricNames.hpp"
#include "order/BuySell.hpp"

namespace tradeData {

    using MetricValue = std::vector<std::pair<MetricName, double>>;

    class SignalEngine {
        using metricUp = std::unique_ptr<metrics::Metric>;
        using limitPair = std::pair<double, double>;

        const std::string symbol;
        execution::OrderExecution& orderExecutor;

        uint64_t latestUpdateId{};
        std::unique_ptr<OrderBookLevel> lastOrderBookLevelUp = nullptr;

        std::unordered_map<MetricName, metricUp> metricCalculators;
        std::unordered_map<MetricName, limitPair> metricLimits;
        uint32_t metricCount{};

        void updateMetrics(const OrderBookLevel & orderBookLevel) const;

        std::optional<Order::BuySell> orderConditionsMet();

    public:
        SignalEngine(std::string  symbol, execution::OrderExecution& orderExecutor_);

        void processData(const OrderBookLevel& orderBookLevel);
        void addMetric(MetricName metricName, metricUp metric, const limitPair &limits);
        std::expected<bool, std::string> removeMetric(MetricName metricName);
        void updateLimit(MetricName metricName, limitPair limits);

        OrderBookLevel getLastProcessedData() const;
    };
}
