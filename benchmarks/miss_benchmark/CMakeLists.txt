project(miss_benchmark LANGUAGES CXX)

set(miss_benchmark_sources
    packet_counter.h
    packet_counter.cpp
    main.cpp
)

add_executable(${PROJECT_NAME} ${miss_benchmark_sources})
target_link_libraries(${PROJECT_NAME} bpf_core)

if (CMAKE_CXX_COMPILER_ID MATCHES GNU|Clang)
    target_compile_options(${PROJECT_NAME} PUBLIC -Wall -Wextra -Wpedantic -Werror -Wno-unused-result -fPIC)
endif()
