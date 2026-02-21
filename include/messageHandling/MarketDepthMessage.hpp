#pragma once

#include "MarketOrder.hpp"
#include <vector>

class MarketDepthMessage {
    std::vector<MarketOrder> bids;
    std::vector<MarketOrder> asks;

    static std::vector<std::string> splitStringOnDelim(const std::string& s, const char delim);

public:
    explicit MarketDepthMessage(const std::string& message);

    void printMarketDepth();
};

static void printVec(std::vector<MarketOrder>& orders);