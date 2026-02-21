#include "positionManagement/SimplePositionManager.hpp"

namespace positionManagement {
    void SimplePositionManager::updatePositionWithTrade(const BuySell buySell, const double quantity, const double price) {
        const double signedQty = buySell == BuySell::BUY ? quantity: -quantity;

        // If same direction or flat
        if (position == 0 || (position > 0) == (signedQty > 0)) {

            double newPosition = position + signedQty;
            averagePrice =
                (position * averagePrice + signedQty * price)
                / newPosition;

            position = newPosition;
            return;
        }

        // Opposite direction — reducing or flipping

        const double closingQty = std::min(std::abs(position), std::abs(signedQty));

        realisedPnl += closingQty * (price - averagePrice)
                        * (position > 0 ? 1 : -1);

        position += signedQty;

        if (position == 0) {
            averagePrice = 0.0;
        } else if (position > 0 == signedQty > 0) {
            averagePrice = price; // flipped direction
        }
    }
}
