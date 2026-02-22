cmake_minimum_required(VERSION 3.31.6)

project(Market-Data-Project LANGUAGES CXX)

enable_testing()
set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-std=c++23)

message("C++ Compiler ID: ${CMAKE_CXX_COMPILER_ID}")
message("C++ Compiler Version: ${CMAKE_CXX_COMPILER_VERSION}")
message("C++ Compiler Path: ${CMAKE_CXX_COMPILER}")

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION OFF)

string(APPEND CMAKE_CXX_FLAGS " -march=native -g") # may need -pthread

find_package(OpenSSL CONFIG REQUIRED)

find_package(Boost REQUIRED COMPONENTS headers)

set(LIB_SOURCES
        src/WebConnection/WebSocketClient.cpp
        src/MessageHandling/MarketOrder.cpp
        src/MessageHandling/MarketDepthMessage.cpp
        src/MessageHandling/OrderFactory.cpp
        src/MessageHandling/OrderBookLevel.cpp
        include/tradeData/SignalEngine.hpp
        include/tradeData/metric/Metric.hpp
        src/tradeData/metric/buySellMetric/BidAskVolumeRatio.cpp
        include/tradeData/metric/buySellMetric/BidAskVolumeRatio.hpp
        src/tradeData/metric/sizingMetrics/MidPriceRealisedVolatility.cpp
        include/execution/OrderExecution.hpp
        src/execution/OrderExecutionSim.cpp
        include/execution/OrderExecutionSim.hpp
        src/positionManagement/PositionManager.cpp
        include/positionManagement/PositionManager.hpp
        src/positionManagement/SimplePositionManager.cpp
        include/positionManagement/SimplePositionManager.hpp
        src/tradeData/metric/buySellMetric/buySellMetric.cpp
        include/tradeData/metric/buySellMetric/buySellMetric.hpp
        src/tradeData/metric/sizingMetrics/SizingMetric.cpp
        include/tradeData/metric/sizingMetric/SizingMetric.hpp
)

set(HEADERS
        include/MessageHandling/MarketDepthMessage.hpp
        include/MessageHandling/MarketOrder.hpp
        include/MessageHandling/OrderFactory.hpp
        include/MessageHandling/OrderBookLevel.hpp
        include/MessageQueue/MessageQueue.hpp
        include/MessageQueue/MessageQueueConsumer.hpp
        include/tradeData/SignalEngine.hpp
        include/tradeData/metric/Metric.hpp
        src/tradeData/metric/buySellMetric/BidAskVolumeRatio.cpp
        include/tradeData/metric/buySellMetric/BidAskVolumeRatio.hpp
        src/Order/Order.cpp
        include/Order/Order.hpp
        include/Order/BuySell.hpp
        include/Order/OrderType.hpp
        include/Order/OrderExpiry.hpp
        include/tradeData/metric/MetricNames.hpp
        include/tradeData/metric/sizingMetric/MidPriceRealisedVolatility.hpp
        src/TradeData/SignalEngine.cpp
        include/execution/OrderExecution.hpp
        src/execution/OrderExecutionSim.cpp
        include/execution/OrderExecutionSim.hpp
        src/positionManagement/PositionManager.cpp
        include/positionManagement/PositionManager.hpp
        src/positionManagement/SimplePositionManager.cpp
        include/positionManagement/SimplePositionManager.hpp
        src/tradeData/metric/buySellMetric/buySellMetric.cpp
        include/tradeData/metric/buySellMetric/buySellMetric.hpp
        src/tradeData/metric/sizingMetrics/SizingMetric.cpp
        include/tradeData/metric/sizingMetric/SizingMetric.hpp
)

add_library(market_data_lib
        ${LIB_SOURCES}
        ${HEADERS}
)

target_include_directories(market_data_lib
        PUBLIC
        ${PROJECT_SOURCE_DIR}/include
        ${Boost_INCLUDE_DIRS}
)

target_link_libraries(market_data_lib
        PUBLIC
        OpenSSL::SSL
        OpenSSL::Crypto
)

# app-only sources: main.cpp
add_executable(market_data
        src/main.cpp
)

target_link_libraries(market_data
        PRIVATE
        market_data_lib
)

enable_testing()
add_subdirectory(tests)