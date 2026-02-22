#pragma once

#include "OrderExecution.hpp"

namespace execution {
    class OrderExecutionSim final : public OrderExecution {
    public:
        explicit OrderExecutionSim(PM& pm) : OrderExecution(pm) {}

        void submitOrder(BuySell, double quantity, double price) override;

        void resetPosition(OrderBookLevel orderBookLevel) override;
    };
}
