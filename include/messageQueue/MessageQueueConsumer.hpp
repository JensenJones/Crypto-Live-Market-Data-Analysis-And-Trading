#pragma once

namespace messageQueue {
    template<typename Queue>
    concept QueueConcept =
        requires(Queue q, std::stop_token st) {
            // Must expose valueType
            typename Queue::valueType;

            // Must support dequeue(stop_token) returning std::optional<valueType>
            { q.dequeue(st) } -> std::same_as<std::optional<typename Queue::valueType>>;
        };

    template<QueueConcept Queue, typename SigEng>
    class MessageQueueConsumer {
        Queue& queue;
        SigEng& signalEngine;
        inline static std::mutex printingMutex{};

        using valueType = Queue::valueType;

        void sendToProcessor(std::optional<valueType> dataOptional);

    public:
        explicit MessageQueueConsumer(Queue &queue, SigEng& signalEngine);

        void operator()(const std::stop_token& stopToken);
    };

    template<QueueConcept Queue, typename Processor>
    MessageQueueConsumer<Queue, Processor>::MessageQueueConsumer(Queue &queue, Processor& signalEngine) :
        queue(queue),
        signalEngine(signalEngine) {
    }

    template<QueueConcept Queue, typename Processor>
    void MessageQueueConsumer<Queue, Processor>::sendToProcessor(std::optional<valueType> dataOptional) {
        if (dataOptional) {
            std::unique_lock lock(printingMutex);
            signalEngine.processData(dataOptional.value());
        }
    }

    template<QueueConcept Queue, typename Processor>
    void MessageQueueConsumer<Queue, Processor>::operator()(const std::stop_token &stopToken) {
        while (!stopToken.stop_requested()) {
            sendToProcessor(queue.dequeue(stopToken));
        }
    }
}
