cmake_minimum_required(VERSION 3.31.6)

project(Market-Data-Project LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION OFF)

string(APPEND CMAKE_CXX_FLAGS " -march=native -g") # may need -pthread

find_package(OpenSSL CONFIG REQUIRED)

find_package(Boost REQUIRED COMPONENTS headers)

set(LIB_SOURCES
        src/WebConnection/WebSocketClient.cpp
        src/MessageHandling/Order.cpp
        src/MessageHandling/MarketDepthMessage.cpp
        src/MessageHandling/OrderFactory.cpp
        src/MessageHandling/TopOfBook.cpp
        include/DataProcessing/DataProcessor.hpp
        include/DataProcessing/Metrics/Metric.hpp
        src/DataProcessing/Metrics/BidAskVolumeRatio.cpp
        include/DataProcessing/Metrics/BidAskVolumeRatio.hpp
        src/DataProcessing/Metrics/MidPriceRealisedVolatility.cpp
)

set(HEADERS
        include/MessageHandling/MarketDepthMessage.hpp
        include/MessageHandling/Order.hpp
        include/MessageHandling/OrderFactory.hpp
        include/MessageHandling/TopOfBook.hpp
        include/MessageQueue/MessageQueue.hpp
        include/MessageQueue/MessageQueueConsumer.hpp
        include/DataProcessing/DataProcessor.hpp
        include/DataProcessing/Metrics/Metric.hpp
        src/DataProcessing/Metrics/BidAskVolumeRatio.cpp
        include/DataProcessing/Metrics/BidAskVolumeRatio.hpp
        src/Order/Order.cpp
        include/Order/Order.hpp
        include/Order/OrderSide.hpp
        include/Order/OrderType.hpp
        include/Order/OrderExpiry.hpp
        include/DataProcessing/Metrics/MetricNames.hpp
        include/DataProcessing/Metrics/MidPriceRealisedVolatility.hpp
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