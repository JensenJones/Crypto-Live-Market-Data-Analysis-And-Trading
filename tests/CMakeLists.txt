cmake_minimum_required(VERSION 3.31.6)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(GTest REQUIRED)

include_directories(${GTEST_INCLUDE_DIRS})

add_executable(Tests
        MessageQueue/MessageQueueTests.cpp
        DataProcessing/Metrics/BidAskVolumeRatioTests.cpp
        DataProcessing/Metrics/MidPriceRealisedVolatilityTests.cpp
        DataProcessing/SignalEngineTests.cpp
)

target_compile_options(Tests PRIVATE -std=c++23)

set_target_properties(Tests PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON
)

# Force C++23 for this target
target_compile_features(Tests
        PUBLIC
        cxx_std_23
)

target_link_libraries(Tests
        PRIVATE
        market_data_lib
        GTest::gtest
        GTest::gtest_main
        GTest::gmock
        Threads::Threads
)

target_include_directories(Tests
        PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

target_include_directories(Tests
        PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

add_test(NAME Tests COMMAND Tests)