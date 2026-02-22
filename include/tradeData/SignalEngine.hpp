#pragma once

#include <expected>
#include <string>
#include <utility>
#include <optional>
#include <unordered_map>

#include "execution/OrderExecution.hpp"
#include "metric/buySellMetric/BuySellMetric.hpp"
#include "metric/sizingMetric/SizingMetric.hpp"
#include "tradeData/metric/Metric.hpp"
#include "tradeData/metric/MetricNames.hpp"
#include "order/BuySell.hpp"

namespace tradeData {

    class SignalEngine {
        using buySellMetricUp = std::unique_ptr<metrics::BuySellMetric>;
        using sizingMetricUp = std::unique_ptr<metrics::SizingMetric>;
        using doublePair = std::pair<double, double>;

        const std::string symbol;
        execution::OrderExecution& orderExecutor;

        uint64_t latestUpdateId{};
        std::unique_ptr<OrderBookLevel> lastOrderBookLevelUp = nullptr;

        std::unordered_map<MetricName, buySellMetricUp> buySellMetricCalculators;

        std::unordered_map<MetricName, sizingMetricUp> sizingMetricCalculators;

        uint32_t buySellMetricCount{};
        uint32_t sizingMetricCount{};

        void updateMetrics(const OrderBookLevel & orderBookLevel) const;

        std::optional<Order::BuySell> orderConditionsMet();

        double calculateOrderSize() const;
    public:
        SignalEngine(std::string symbol, execution::OrderExecution& orderExecutor_);


        void processData(const OrderBookLevel& orderBookLevel);

        void addBuySellMetric(MetricName metricName, buySellMetricUp metric);
        void updateBuySellMetricDecision(MetricName metricName, metrics::BuySellDecision buySellDecision);

        void addSizingMetric(MetricName metricName, sizingMetricUp metric);
        void updateSizingMetricDecision(MetricName metricName, metrics::SizingDecision sizingDecision);

        std::expected<bool, std::string> removeMetric(MetricName metricName);

        OrderBookLevel getLastProcessedData() const;
    };
}
