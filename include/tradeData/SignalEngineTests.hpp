#pragma once

#include <expected>
#include <string>
#include <utility>
#include <optional>
#include <unordered_map>

#include "metrics/Metric.hpp"
#include "metrics/MetricNames.hpp"
#include "order/BuySell.hpp"

namespace tradeData {

    using MetricValue = std::vector<std::pair<MetricName, double>>;

    class SignalEngine {
        using metricUp = std::unique_ptr<metrics::Metric>;
        using limitPair = std::pair<double, double>;

        const std::string symbol;

        uint64_t latestUpdateId{};
        std::unordered_map<MetricName, metricUp> metricCalculators;
        std::unordered_map<MetricName, limitPair> metricLimits;
        uint32_t metricCount{};

        void updateMetrics(const OrderBookLevel & orderBookLevel) const;

        std::optional<Order::BuySell> orderConditionsMet();

    public:
        explicit SignalEngine(std::string symbol);

        void processData(const OrderBookLevel& orderBookLevel);
        void addMetric(MetricName metricName, metricUp metric, const limitPair &limits);
        std::expected<bool, std::string> removeMetric(MetricName metricName);
        void updateLimit(MetricName metricName, limitPair limits);
    };
}
