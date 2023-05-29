project(common_benchmark LANGUAGES CXX)

set(common_benchmark_sources
    bench.hpp
    bench_bpf.cpp
    bench_mem.cpp
    bench_str.cpp
    bench_std.cpp
    main.cpp
)

add_executable(${PROJECT_NAME} ${common_benchmark_sources})
target_link_libraries(${PROJECT_NAME} benchmark::benchmark_main bpf_core)

if (CMAKE_CXX_COMPILER_ID MATCHES GNU|Clang)
    target_compile_options(${PROJECT_NAME} PUBLIC -Wall -Wextra -Wpedantic -Werror -Wno-unused-result -fPIC)
endif()
