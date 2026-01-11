#pragma once
#include <string>
#include <utility>

#include "Metrics/Metric.hpp"
#include "../MessageHandling/TopOfBook.hpp"
#include "Metrics/MetricNames.hpp"

namespace dataProcessing {

    template<typename Queue>
    concept EnqueueQueue = requires(Queue& q) {
        q.enqueue(std::declval<typename Queue::valueType>());
    };

    template<EnqueueQueue Queue>
    class DataProcessor {
        using metricUp = std::unique_ptr<metrics::Metric>;
        using limitPair = std::pair<double, double>;

        const std::string symbol;

        uint64_t latestUpdateId{};
        std::unordered_map<MetricName, metricUp> metricCalculators;
        std::unordered_map<MetricName, limitPair> metricLimits;
        const Queue& metricDataOutgoingQueue;

        using valueType = Queue::valueType;

        void updateMetrics(const TopOfBook & topOfBook) const;

    public:
        DataProcessor(std::string symbol, Queue& metricOutgoingDataQueue);

        void processData(const TopOfBook& topOfBook);
        void addMetric(MetricName metricName, metricUp metric);
        void addMetric(MetricName metricName, metricUp metric, const limitPair &limits);
        void updateLimit(MetricName metricName, limitPair limits);
    };

    template<EnqueueQueue Queue>
    DataProcessor<Queue>::DataProcessor(std::string symbol, Queue &metricOutgoingDataQueue) :
        symbol(std::move(symbol)), metricDataOutgoingQueue(metricOutgoingDataQueue) {
    }

    template<EnqueueQueue Queue>
    void DataProcessor<Queue>::updateMetrics(const TopOfBook &topOfBook) const {
        for (const auto &metric: metricCalculators | std::views::values) {
            metric->update(topOfBook);
        }
    }

    template<EnqueueQueue Queue>
    void DataProcessor<Queue>::processData(const TopOfBook& topOfBook) {
        if (const uint64_t newLatestUpdateId = topOfBook.getUpdateId();
            newLatestUpdateId > latestUpdateId) {
            updateMetrics(topOfBook);
            latestUpdateId = newLatestUpdateId;

            std::vector<double> outgoingMetrics;

        }
    }

    template<EnqueueQueue Queue>
    void DataProcessor<Queue>::addMetric(const MetricName metricName, metricUp metric) {
        metricCalculators[metricName] = std::move(metric);
    }

    template<EnqueueQueue Queue>
    void DataProcessor<Queue>::addMetric(const MetricName metricName, metricUp metric, const limitPair &limits) {
        metricCalculators[metricName] = std::move(metric);
        metricLimits[metricName] = limits;
    }

    template<EnqueueQueue Queue>
    void DataProcessor<Queue>::updateLimit(MetricName metricName, limitPair limits) {
        metricLimits[metricName] = std::move(limits);
    }
}
