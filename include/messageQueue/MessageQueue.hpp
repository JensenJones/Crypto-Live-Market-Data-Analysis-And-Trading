#pragma once

#include <array>
#include <concepts>
#include <optional>
#include <condition_variable>

namespace messageQueue {
    template<typename T, size_t CAPACITY>
    class MessageQueue {
        std::array<std::optional<T>, CAPACITY> data{};

        size_t head{};
        size_t tail{};
        size_t size{};

        std::mutex mutex;
        std::condition_variable_any conditionVariable;

    public:
        using valueType = T;

        template<typename U>
        requires std::constructible_from<T, U&&>
        void enqueue(U&& message);
        std::optional<T> dequeue(const std::stop_token& stopToken);

        [[nodiscard]] bool isEmpty() const;
        [[nodiscard]] bool isFull() const;
    };

    template<typename T, size_t CAPACITY>
    template<typename U> requires std::constructible_from<T, U &&>
    void MessageQueue<T, CAPACITY>::enqueue(U &&message) {
        std::unique_lock lock{mutex};

        while (isFull()) {
            conditionVariable.wait(lock);
        }

        data[head].emplace(std::forward<U>(message));
        head = (head + 1) % CAPACITY;
        ++size;

        conditionVariable.notify_all();
    }

    template<typename T, size_t CAPACITY>
    std::optional<T> MessageQueue<T, CAPACITY>::dequeue(const std::stop_token& stopToken) {
        std::unique_lock lock{mutex};

        conditionVariable.wait(lock, stopToken, [&] {
            return size > 0;
        });

        if (stopToken.stop_requested() && size == 0) {
            return std::nullopt;
        }

        auto result = std::move(data[tail]);
        data[tail].reset();
        tail = (tail + 1) % CAPACITY;
        --size;

        conditionVariable.notify_all();

        return result;
    }

    template<typename T, size_t CAPACITY>
    bool MessageQueue<T, CAPACITY>::isEmpty() const {
        return size == 0;
    }

    template<typename T, size_t CAPACITY>
    bool MessageQueue<T, CAPACITY>::isFull() const {
        return size == CAPACITY;
    }
}
