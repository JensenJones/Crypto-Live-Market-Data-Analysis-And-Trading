#pragma once
#include "messageHandling/OrderBookLevel.hpp"
#include "order/BuySell.hpp"
#include "positionManagement/PositionManager.hpp"

namespace execution {
    using PM = positionManagement::PositionManager;
    using BuySell = Order::BuySell;

    class OrderExecution {
    protected:
        PM& positionManager;

        explicit OrderExecution(PM& pm) : positionManager{pm} {}

    public:
        virtual ~OrderExecution() = default;
        virtual void submitOrder(BuySell, double quantity, double price) = 0;
        virtual void resetPosition(OrderBookLevel orderBookLevel) = 0;
    };
}
