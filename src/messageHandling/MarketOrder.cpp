#include "../../include/messageHandling/MarketOrder.hpp"

MarketOrder::MarketOrder() : price{ 0.0 }, quantity{ 0.0 } {}

MarketOrder::MarketOrder(const double price, const double quantity) : price{ price }, quantity{ quantity } {}

void MarketOrder::set_price(const double newPrice) {
    this->price = newPrice;
}


void MarketOrder::set_quantity(const double newQuantity) {
    this->quantity = newQuantity;
}

double MarketOrder::getPrice() const { return price; }

double MarketOrder::getQuantity() const { return quantity; }
