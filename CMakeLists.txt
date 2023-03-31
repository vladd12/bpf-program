cmake_minimum_required(VERSION 3.5)
project(bpf_test LANGUAGES CXX)

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

# Collecting sources of project
set(files_h
    include/bpf_wrap.h
    include/code_formater.h
    include/fast_file.h
    include/utils.h
)
set(files_cpp
    src/bpf_wrap.cpp
    src/code_formater.cpp
    src/fast_file.cpp
    src/main.cpp
    src/utils.cpp
)

# Adding paths of headers to include
include_directories("${PROJECT_SOURCE_DIR}/include")

# Compilation project
add_executable(${PROJECT_NAME} ${files_h} ${files_cpp})

# Linking libraries
target_link_libraries(${PROJECT_NAME} PUBLIC ${LIBBCC_LIBRARIES})
target_include_directories(${PROJECT_NAME} PUBLIC ${LIBBCC_INCLUDE_DIR})

# Building benchmarks with project
if (BUILD_BENCHMARKS)
    include(get_gbench)
    add_subdirectory(benchmarks)
endif()

# Some compilation flags
if (CMAKE_CXX_COMPILER_ID MATCHES GNU|Clang)
    add_compile_options(-Wall -Wextra -Wpedantic -Werror -Wno-unused-result -fPIC)
endif()

# Installing eBPF sources in directory with program
set(BPF_SOURCE_DIR "${PROJECT_SOURCE_DIR}/bpf")
install(
  FILES ${BPF_SOURCE_DIR}/ethernet-parse.c
  DESTINATION ${CMAKE_BINARY_DIR}/bpf
)
