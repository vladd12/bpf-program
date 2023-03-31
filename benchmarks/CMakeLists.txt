project(bpf_bechmarks LANGUAGES CXX)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} benchmark::benchmark_main)
