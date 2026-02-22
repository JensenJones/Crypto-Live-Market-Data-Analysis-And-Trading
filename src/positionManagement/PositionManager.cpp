#include "positionManagement/PositionManager.hpp"

namespace positionManagement {
    double PositionManager::getPosition() const { return position; }
    double PositionManager::getRealisedPnl() const { return realisedPnl; }
    double PositionManager::getAveragePrice() const { return averagePrice; }
    double PositionManager::getPeakCashUsed() const { return peakCashUsed; }
}
