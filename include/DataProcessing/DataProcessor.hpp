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
        using metricUP = std::unique_ptr<metrics::Metric>;
        using limitPair = std::pair<double, double>;
        const std::string symbol;

        uint64_t latestUpdateId{};
        std::unordered_map<MetricName, metricUP> metrics;
        std::unordered_map<MetricName, limitPair> metricLimits;
        const Queue& tradeQueue;

        using valueType = Queue::valueType;

        void updateGreeks(const TopOfBook & topOfBook) const;

        [[nodiscard]] double calcBidAmount() const;
        [[nodiscard]] double calcAskAmount() const;

        void checkConditions() const;
    public:
        DataProcessor(std::string symbol, Queue& tradeQueue);

        void processData(const TopOfBook& topOfBook);
        void addMetric(MetricName greekName, metricUP greek);
        void addMetric(MetricName greekName, metricUP greek, limitPair limits);
        void updateLimit(MetricName greekName, limitPair limits);
    };

    template<EnqueueQueue Queue>
    DataProcessor<Queue>::DataProcessor(std::string symbol, Queue &tradeQueue) :
        symbol(std::move(symbol)), tradeQueue(tradeQueue) {
    }

    template<EnqueueQueue Queue>
    void DataProcessor<Queue>::updateGreeks(const TopOfBook &topOfBook) const {
        for (const auto &greek: metrics | std::views::values) {
            greek->update(topOfBook);
        }
    }

    template<EnqueueQueue Queue>
    double DataProcessor<Queue>::calcBidAmount() const {
        return 0.0;
    }

    template<EnqueueQueue Queue>
    double DataProcessor<Queue>::calcAskAmount() const {
        return 0.0;
    }

    template<EnqueueQueue Queue>
    void DataProcessor<Queue>::checkConditions() const {
        // if (calcBidAmount()) {
        //     // TODO: place bid order
        //     tradeQueue.enqueue(Order());
        // } else {
        //     if (calcAskAmount()) {
        //         // TODO: place ask order
        //         tradeQueue.enqueue(Order());
        //     }
        // }
    }

    template<EnqueueQueue Queue>
    void DataProcessor<Queue>::processData(const TopOfBook& topOfBook) {
        if (uint64_t newLatestUpdateId = topOfBook.getUpdateId();
            newLatestUpdateId > latestUpdateId) {
            updateGreeks(topOfBook);
            latestUpdateId = newLatestUpdateId;
        }
    }

    template<EnqueueQueue Queue>
    void DataProcessor<Queue>::addMetric(const MetricName greekName, metricUP greek) {
        metrics[greekName] = std::move(greek);
    }

    template<EnqueueQueue Queue>
    void DataProcessor<Queue>::addMetric(MetricName greekName, metricUP greek, limitPair limits) {
        metrics[greekName] = std::move(greek);
        metricLimits[greekName] = limits;
    }

    template<EnqueueQueue Queue>
    void DataProcessor<Queue>::updateLimit(MetricName greekName, limitPair limits) {
        metricLimits[greekName] = std::move(limits);
    }
}
