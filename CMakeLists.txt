cmake_minimum_required(VERSION 3.5)
project(bpf_app LANGUAGES CXX)

# Initial project settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

# Project options
option(BUILD_BENCHMARKS "Building benchmarks" ON)

# Setting project build type, if not specified
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Build type" FORCE)
    message(STATUS "CMAKE_BUILD_TYPE is not specified. Using default: ${CMAKE_BUILD_TYPE}")
endif()

# Finding libbcc
find_package(bcc REQUIRED)
if (NOT LIBBCC_FOUND)
    message(FATAL_ERROR "Not found libbcc, please install it")
else()
    message(STATUS "libbcc was found:
    Include path: ${LIBBCC_INCLUDE_DIR}
    Dynamic lib path: ${LIBBCC_LIBRARIES}
    Static lib path: ${LIBBCC_STATIC_LIBRARIES}")
endif()

# Finding libpcap
find_package(pcap REQUIRED)
if (NOT LIBPCAP_FOUND)
    message(FATAL_ERROR "Not found libpcap, please install it")
else()
    message(STATUS "libpcap was found:
    Include path: ${LIBPCAP_INCLUDE_DIR}
    Dynamic lib path: ${LIBPCAP_LIBRARIES}
    Static lib path: ${LIBPCAP_STATIC_LIBRARIES}")
endif()

# Collecting sources of project
set(bpf_core_h
    include/bpf_exec.h
    include/byte_op.h
    include/code_formater.h
    include/fast_file.h
    include/iec_parser.h
    include/utils.h
    include/net/sock.h
    include/net/netdriver.h
    include/net/validator.h
)
set(bpf_core_cpp
    src/bpf_exec.cpp
    src/byte_op.cpp
    src/code_formater.cpp
    src/fast_file.cpp
    src/iec_parser.cpp
    src/utils.cpp
    src/net/sock.cpp
    src/net/netdriver.cpp
    src/net/validator.cpp
)
set(bpf_app_cpp src/main.cpp)

# Adding paths of headers to include
include_directories("${PROJECT_SOURCE_DIR}/include")

# Compilation static library as part of project
add_library(bpf_core STATIC ${bpf_core_h} ${bpf_core_cpp})

# Linking libraries
target_link_libraries(bpf_core PUBLIC ${LIBBCC_LIBRARIES} ${LIBPCAP_LIBRARIES})
target_include_directories(bpf_core PUBLIC ${LIBBCC_INCLUDE_DIR} ${LIBPCAP_INCLUDE_DIR})
# Compilation flags
if (CMAKE_CXX_COMPILER_ID MATCHES GNU|Clang)
    target_compile_options(bpf_core PUBLIC -Wall -Wextra -Wpedantic -Werror -Wno-unused-result -fPIC)
endif()

# Compilation application
add_executable(${PROJECT_NAME} ${bpf_app_cpp})
target_link_libraries(${PROJECT_NAME} PUBLIC bpf_core)
# Compilation flags
if (CMAKE_CXX_COMPILER_ID MATCHES GNU|Clang)
    target_compile_options(${PROJECT_NAME} PUBLIC -Wall -Wextra -Wpedantic -Werror -Wno-unused-result -fPIC)
endif()

# Building benchmarks with project
if (BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()

# Installing eBPF sources in directory with program
set(BPF_SOURCE_DIR "${PROJECT_SOURCE_DIR}/bpf")
install(
  FILES ${BPF_SOURCE_DIR}/ethernet-parse.c
  DESTINATION ${CMAKE_BINARY_DIR}/bpf
)
