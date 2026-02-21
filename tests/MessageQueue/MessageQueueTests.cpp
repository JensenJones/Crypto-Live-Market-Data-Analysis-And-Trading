#include <gtest/gtest.h>
#include <stop_token>
#include <thread>
#include <algorithm>

#include "../../include/messageQueue/MessageQueue.hpp"

class MessageQueueTest : public testing::Test {
protected:
    constexpr static size_t QUEUE_CAPACITY = 3;
    messageQueue::MessageQueue<int, QUEUE_CAPACITY> queue;
    std::stop_source stopSource;
    std::stop_token stopToken;

    template<typename... Args>
    void populateQueue(Args&&... args) {
        (queue.enqueue(std::forward<Args>(args)), ...);
    }

    void SetUp() override {
        stopToken = stopSource.get_token();
    }
};

TEST_F(MessageQueueTest, shouldEnqueueAndDequeueSingleValue) {
    populateQueue(22);
    const int value = queue.dequeue(stopToken).value();
    EXPECT_EQ(value, 22);
}

TEST_F(MessageQueueTest, shouldEnqueueAndDequeueMultipleMessagesFIFO) {
    populateQueue(1, 2, 3);

    EXPECT_EQ(queue.isFull(), true);

    auto first = queue.dequeue(stopToken).value();
    auto second = queue.dequeue(stopToken).value();
    auto third = queue.dequeue(stopToken).value();

    EXPECT_EQ(first, 1);
    EXPECT_EQ(second, 2);
    EXPECT_EQ(third, 3);
}

TEST_F(MessageQueueTest, shouldBlockOrdersExceedingCapacity) {
    populateQueue(4, 5, 6);

    EXPECT_EQ(queue.isFull(), true);

    std::jthread queueExcessDataThread([&] {populateQueue(7);});
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    EXPECT_EQ(queue.isFull(), true);
    EXPECT_EQ(queueExcessDataThread.joinable(), true); // Enqueue is blocked

    const auto four = queue.dequeue(stopToken).value();
    const auto five = queue.dequeue(stopToken).value();
    const auto six = queue.dequeue(stopToken).value();
    const auto seven = queue.dequeue(stopToken).value();

    EXPECT_EQ(four, 4);
    EXPECT_EQ(five, 5);
    EXPECT_EQ(six, 6);
    EXPECT_EQ(seven, 7);
    EXPECT_EQ(queue.isEmpty(), true);
}

TEST_F(MessageQueueTest, shouldBlockDequeueUntilItemAvailable) {
    std::optional<int> result;
    std::mutex m;
    std::condition_variable cv;
    bool done = false;

    std::jthread consumerThread([&](std::stop_token) {
        auto value = queue.dequeue(stopToken);
        {
            std::lock_guard lock(m);
            result = std::move(value);
            done = true;
        }
        cv.notify_one();
    });

    // Give the consumer a moment to start and (likely) block
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    // Queue is still empty, so we expect the consumer to still be waiting
    {
        std::lock_guard lock(m);
        EXPECT_FALSE(done);
        EXPECT_FALSE(result.has_value());
    }

    // Now enqueue a value; this should unblock the consumer
    queue.enqueue(99);

    std::unique_lock lk(m);
    ASSERT_TRUE(cv.wait_for(lk, std::chrono::milliseconds(500), [&] { return done; }));

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 99);
}

TEST_F(MessageQueueTest, shouldReturnNulloptWhenStoppedAndEmpty) {
    std::optional<int> result;
    std::mutex m;
    std::condition_variable cv;
    bool done = false;

    std::jthread consumerThread([&](std::stop_token) {
        auto value = queue.dequeue(stopToken);
        {
            std::lock_guard lock(m);
            result = std::move(value);
            done = true;
        }
        cv.notify_one();
    });

    // Let the consumer block on the empty queue
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    // Request stop; dequeue should wake and return std::nullopt
    stopSource.request_stop();

    std::unique_lock lk(m);
    ASSERT_TRUE(cv.wait_for(lk, std::chrono::milliseconds(500), [&] { return done; }));

    EXPECT_FALSE(result.has_value());
}

TEST_F(MessageQueueTest, shouldWrapAroundCorrectly) {
    // Capacity is 3; push and pop to force head/tail to wrap
    populateQueue(1, 2, 3);

    EXPECT_TRUE(queue.isFull());

    auto first = queue.dequeue(stopToken).value();
    EXPECT_EQ(first, 1);

    // Now there is 1 slot free; enqueue another element, this should wrap
    queue.enqueue(4);
    EXPECT_TRUE(queue.isFull());

    auto second = queue.dequeue(stopToken).value();
    auto third  = queue.dequeue(stopToken).value();
    auto fourth = queue.dequeue(stopToken).value();

    EXPECT_EQ(second, 2);
    EXPECT_EQ(third, 3);
    EXPECT_EQ(fourth, 4);
    EXPECT_TRUE(queue.isEmpty());
}

TEST_F(MessageQueueTest, shouldReportEmptyAndFullCorrectly) {
    EXPECT_TRUE(queue.isEmpty());
    EXPECT_FALSE(queue.isFull());

    queue.enqueue(10);
    EXPECT_FALSE(queue.isEmpty());
    EXPECT_FALSE(queue.isFull());

    queue.enqueue(20);
    queue.enqueue(30);
    EXPECT_TRUE(queue.isFull());

    // Dequeue one → not empty, not full
    (void)queue.dequeue(stopToken);
    EXPECT_FALSE(queue.isEmpty());
    EXPECT_FALSE(queue.isFull());

    // Dequeue remaining → empty again
    (void)queue.dequeue(stopToken);
    (void)queue.dequeue(stopToken);
    EXPECT_TRUE(queue.isEmpty());
    EXPECT_FALSE(queue.isFull());
}

TEST_F(MessageQueueTest, shouldHandleMultipleProducersAndConsumers) {
    // Use a slightly larger local queue so we’re not constantly blocking
    constexpr size_t LocalCapacity = 16;
    messageQueue::MessageQueue<int, LocalCapacity> localQueue;

    constexpr int producerCount = 3;
    constexpr int consumerCount = 3;
    constexpr int itemsPerProducer = 100;
    const int totalItems = producerCount * itemsPerProducer;

    std::stop_source localStopSource;
    std::stop_token localStopToken = localStopSource.get_token();

    std::atomic produced{0};
    std::atomic consumed{0};

    std::mutex collectedMutex;
    std::vector<int> collected;
    collected.reserve(totalItems);

    std::vector<std::jthread> threads;

    // Producers
    threads.reserve(producerCount);
    for (int p = 0; p < producerCount; ++p) {
        threads.emplace_back([&, p](std::stop_token) {
            for (int i = 0; i < itemsPerProducer; ++i) {
                int value = p * itemsPerProducer + i;
                localQueue.enqueue(value);
                ++produced;
            }
        });
    }

    // Consumers
    for (int c = 0; c < consumerCount; ++c) {
        threads.emplace_back([&](std::stop_token) {
            while (true) {
                auto value = localQueue.dequeue(localStopToken);
                if (!value.has_value()) {
                    break; // stop requested and queue drained
                }

                {
                    std::lock_guard lock(collectedMutex);
                    collected.push_back(*value);
                }

                if (++consumed >= totalItems) {
                    localStopSource.request_stop();
                }
            }
        });
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    // jthreads join on destruction when leaving scope

    EXPECT_EQ(produced.load(), totalItems);
    EXPECT_EQ(consumed.load(), totalItems);
    ASSERT_EQ(static_cast<int>(collected.size()), totalItems);

    std::ranges::sort(collected);
    for (int i = 0; i < totalItems; ++i) {
        EXPECT_EQ(collected[i], i);
    }
}