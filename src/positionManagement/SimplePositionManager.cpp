#include "positionManagement/SimplePositionManager.hpp"

namespace positionManagement {
    void SimplePositionManager::updatePositionWithTrade(const BuySell buySell, const double quantity, const double price) {
        const double signedQty = buySell == BuySell::BUY ? quantity: -quantity;

        // If same direction or flat
        if (position == 0 || position > 0 == signedQty > 0) {

            const double newPosition = position + signedQty;
            averagePrice =
                (position * averagePrice + signedQty * price)
                / newPosition;

            cashUsed += std::abs(quantity) * price;

            position = newPosition;
            return;
        }

        // Opposite direction — reducing or flipping

        const double closingQty = std::min(std::abs(position), std::abs(signedQty));

        realisedPnl += closingQty * (price - averagePrice)
                        * (position > 0 ? 1 : -1);

        position += signedQty;
        cashUsed -= closingQty * averagePrice;

        if (position == 0) {
            averagePrice = 0.0;
        } else if (position > 0 == signedQty > 0) {
            averagePrice = price; // flipped direction
        }

        if (signedQty > 0) { // BUY
            cashUsed += quantity * price;
        } else {
            cashUsed -= closingQty * averagePrice;
        }

        peakCashUsed = std::max(peakCashUsed, cashUsed);
    }
}
