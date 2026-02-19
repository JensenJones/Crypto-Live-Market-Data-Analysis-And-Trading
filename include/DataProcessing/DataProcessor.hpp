#pragma once

#include <expected>
#include <string>
#include <utility>
#include <optional>

#include "Metrics/Metric.hpp"
#include "../MessageHandling/TopOfBook.hpp"
#include "Metrics/MetricNames.hpp"
#include "Order/BuySell.hpp"

namespace dataProcessing {

    using MetricValue = std::vector<std::pair<MetricName, double>>;

    class DataProcessor {
        using metricUp = std::unique_ptr<metrics::Metric>;
        using limitPair = std::pair<double, double>;

        const std::string symbol;

        uint64_t latestUpdateId{};
        std::unordered_map<MetricName, metricUp> metricCalculators;
        std::unordered_map<MetricName, limitPair> metricLimits;
        uint32_t metricCount{};

        void updateMetrics(const TopOfBook & topOfBook) const;

        std::optional<Order::BuySell> orderConditionsMet();

    public:
        explicit DataProcessor(std::string symbol);

        void processData(const TopOfBook& topOfBook);
        void addMetric(MetricName metricName, metricUp metric);
        void addMetric(MetricName metricName, metricUp metric, const limitPair &limits);
        std::expected<bool, std::string> removeMetric(MetricName metricName);
        void updateLimit(MetricName metricName, limitPair limits);
    };

    inline DataProcessor::DataProcessor(std::string symbol) : symbol(std::move(symbol)) {}

    void DataProcessor::updateMetrics(const TopOfBook &topOfBook) const {
        for (const auto &metric: metricCalculators | std::views::values) {
            metric->update(topOfBook);
        }
    }

    std::optional<Order::BuySell> DataProcessor::orderConditionsMet() {
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

    void DataProcessor::processData(const TopOfBook& topOfBook) {
        if (const uint64_t newLatestUpdateId = topOfBook.getUpdateId();
            newLatestUpdateId > latestUpdateId) {
            std::cout << topOfBook << '\n';
            updateMetrics(topOfBook);
            latestUpdateId = newLatestUpdateId;

            if (auto buySellOpt = orderConditionsMet()) {
                std::cout << "\tCONDITIONS MET, decision is " << buySellOpt.value() << '\n';
            }
        }
    }

    void DataProcessor::addMetric(const MetricName metricName, metricUp metric) {
        metricCalculators[metricName] = std::move(metric);
        ++metricCount;
    }

    void DataProcessor::addMetric(const MetricName metricName, metricUp metric, const limitPair &limits) {
        metricCalculators[metricName] = std::move(metric);
        metricLimits[metricName] = limits;
        ++metricCount;
    }

    std::expected<bool, std::string> DataProcessor::removeMetric(MetricName metricName) {
        if (!metricCalculators.contains(metricName)) {
            return std::unexpected("Metric never added");
        }

        metricCalculators.erase(metricName);

        if (metricLimits.contains(metricName)) {
            metricLimits.erase(metricName);
        }

        return true;
    }

    void DataProcessor::updateLimit(const MetricName metricName, limitPair limits) {
        metricLimits[metricName] = std::move(limits);
    }
}
