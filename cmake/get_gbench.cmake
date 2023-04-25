include(FetchContent)
# Getting Google Benchmark sources from GitHub repository
FetchContent_Declare(
    benchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG        d572f4777349d43653b21d6c2fc63020ab326db2 # v1.7.1
)

set(BENCHMARK_ENABLE_GTEST_TESTS OFF)
set(BENCHMARK_ENABLE_TESTING OFF)
set(BENCHMARK_ENABLE_INSTALL OFF)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG")
endif()

# Compiling Google Benchmark library
FetchContent_MakeAvailable(benchmark)
