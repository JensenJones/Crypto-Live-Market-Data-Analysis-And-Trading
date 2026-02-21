#pragma once
#include <ostream>

class MarketOrder {
    double price;
    double quantity;

public:
    MarketOrder();
    MarketOrder(double price, double quantity);

    void set_price(double newPrice);
    void set_quantity(double newQuantity);

    [[nodiscard]] double getPrice() const;
    [[nodiscard]] double getQuantity() const;

    friend std::ostream & operator<<(std::ostream &os, const MarketOrder &obj) {
        return os << std::format("{:6.6f} x {:2.6f}", obj.price, obj.quantity);
    }
};
