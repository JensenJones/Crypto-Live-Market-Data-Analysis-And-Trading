#pragma once

#include "DataProcessing/DataProcessor.hpp"

namespace messageQueue {
    template<typename Queue>
    concept QueueConcept =
        requires(Queue q, std::stop_token st) {
            // Must expose valueType
            typename Queue::valueType;

            // Must support dequeue(stop_token) returning std::optional<valueType>
            { q.dequeue(st) } -> std::same_as<std::optional<typename Queue::valueType>>;
        };

    template<typename Processor, typename Msg>
    concept ProcessorConcept = requires(Processor& p, const Msg& msg) {
        { p.processData(msg) };
    };

    template<QueueConcept Queue, typename Processor>
    requires ProcessorConcept<Processor, typename Queue::valueType>
    class MessageQueueConsumer {
        Queue& queue;
        Processor& dataProcessor;
        inline static std::mutex printingMutex{};

        using valueType = Queue::valueType;

        void sendToProcessor(std::optional<valueType> dataOptional);

    public:
        explicit MessageQueueConsumer(Queue &queue, Processor& dataProcessor);

        void operator()(const std::stop_token& stopToken);
    };

    template<QueueConcept Queue, typename Processor>
    requires ProcessorConcept<Processor, typename Queue::valueType>
    MessageQueueConsumer<Queue, Processor>::MessageQueueConsumer(Queue &queue, Processor& dataProcessor) :
        queue(queue),
        dataProcessor(dataProcessor) {
    }

    template<QueueConcept Queue, typename Processor>
    requires ProcessorConcept<Processor, typename Queue::valueType>
    void MessageQueueConsumer<Queue, Processor>::sendToProcessor(std::optional<valueType> dataOptional) {
        if (dataOptional) {
            std::unique_lock lock(printingMutex);
            dataProcessor.processData(dataOptional.value());
        }
    }

    template<QueueConcept Queue, typename Processor>
    requires ProcessorConcept<Processor, typename Queue::valueType>
    void MessageQueueConsumer<Queue, Processor>::operator()(const std::stop_token &stopToken) {
        while (!stopToken.stop_requested()) {
            sendToProcessor(queue.dequeue(stopToken));
        }
    }
}
