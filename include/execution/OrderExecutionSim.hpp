#pragma once

#include "OrderExecution.hpp"

namespace execution {
    class OrderExecutionSim : public OrderExecution {
    public:
        explicit OrderExecutionSim(PM& pm) : OrderExecution(pm) {}

        void submitOrder(BuySell, double quantity, double price) override;
    };
}
