enable_testing()

find_package(GTest REQUIRED)

include_directories(${GTEST_INCLUDE_DIRS})

add_executable(Tests
        MessageQueue/MessageQueueTests.cpp
        DataProcessing/Metrics/BidAskVolumeRatioTests.cpp
        DataProcessing/Metrics/MidPriceRealisedVolatilityTests.cpp
)

target_link_libraries(Tests
        PRIVATE
        market_data_lib
        GTest::gtest
        GTest::gtest_main
        Threads::Threads
)

target_include_directories(Tests
        PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

add_test(NAME Tests COMMAND Tests)