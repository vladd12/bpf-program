project(bpf_bechmarks LANGUAGES CXX)

set(bench_sources
    bench.hpp
    bench_mem.cpp
    bench_str.cpp
    bench_std.cpp
    main.cpp
)

# Extra compilation flags
if (CMAKE_CXX_COMPILER_ID MATCHES GNU|Clang)
    set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wpedantic -Werror -Wno-unused-result -fPIC")
    set(CMAKE_CXX_FLAGS_RELEASE "-O2")
endif()

# Setting project build type, if not specified
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

add_executable(${PROJECT_NAME} ${bench_sources})
target_link_libraries(${PROJECT_NAME} benchmark::benchmark_main)
