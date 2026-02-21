#pragma once
#include "order/BuySell.hpp"

namespace execution {

    class OrderExecution {

    public:
        virtual ~OrderExecution() = default;
        virtual void submitOrder(Order::BuySell, double quantity) = 0;
    };
}

