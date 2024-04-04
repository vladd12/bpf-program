cmake_minimum_required(VERSION 3.5)
project(bpf_app LANGUAGES CXX)

# Initial project settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

# Project options
option(BUILD_BENCHMARKS "Building benchmarks" ON)

# Setting project build type, if not specified
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Build type" FORCE)
  message(STATUS "CMAKE_BUILD_TYPE is not specified. Using default: ${CMAKE_BUILD_TYPE}")
endif()

# Finding libbcc
find_package(bcc REQUIRED)
if(NOT LIBBCC_FOUND)
  message(FATAL_ERROR "Not found libbcc, please install it")
else()
  message(STATUS "libbcc was found:
  Include path: ${LIBBCC_INCLUDE_DIR}
  Dynamic lib path: ${LIBBCC_LIBRARIES}
  Static lib path: ${LIBBCC_STATIC_LIBRARIES}")
endif()

# Finding libpcap
find_package(pcap REQUIRED)
if(NOT LIBPCAP_FOUND)
  message(FATAL_ERROR "Not found libpcap, please install it")
else()
  message(STATUS "libpcap was found:
  Include path: ${LIBPCAP_INCLUDE_DIR}
  Dynamic lib path: ${LIBPCAP_LIBRARIES}
  Static lib path: ${LIBPCAP_STATIC_LIBRARIES}")
endif()

# Collecting sources of project
set(bpf_core_h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/bpf_core/bpf_exec.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/bpf_core/iec/byte_op.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/bpf_core/iec/iec_parser.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/bpf_core/iec/validator.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/bpf_core/net/netdriver.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/bpf_core/net/pcap-driver.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/bpf_core/utils/code_formater.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/bpf_core/utils/fast_file.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/bpf_core/utils/helpers.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/bpf_core/utils/socket.h
)
set(bpf_core_cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/bpf_core/bpf_exec.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/bpf_core/iec/byte_op.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/bpf_core/iec/iec_parser.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/bpf_core/iec/validator.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/bpf_core/net/netdriver.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/bpf_core/net/pcap-driver.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/bpf_core/utils/code_formater.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/bpf_core/utils/fast_file.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/bpf_core/utils/helpers.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/bpf_core/utils/socket.cpp
)
set(bpf_app_cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
)

# Compilation static library as part of project
add_library(bpf_core STATIC ${bpf_core_h} ${bpf_core_cpp})

# Including directories for library
target_include_directories(bpf_core PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include;${LIBBCC_LIBRARIES}>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

# Linking libraries
target_link_libraries(bpf_core PUBLIC ${LIBBCC_LIBRARIES} ${LIBPCAP_LIBRARIES})
target_include_directories(bpf_core PUBLIC ${LIBBCC_INCLUDE_DIR} ${LIBPCAP_INCLUDE_DIR})
# Compilation flags
if(CMAKE_CXX_COMPILER_ID MATCHES GNU|Clang)
  target_compile_options(bpf_core PUBLIC -Wall -Wextra -Wpedantic -Wno-unused-result -fPIC)
endif()

# Compilation application
add_executable(${PROJECT_NAME} ${bpf_app_cpp})
target_link_libraries(${PROJECT_NAME} PUBLIC bpf_core)
# Compilation flags
if(CMAKE_CXX_COMPILER_ID MATCHES GNU|Clang)
  target_compile_options(${PROJECT_NAME} PUBLIC -Wall -Wextra -Wpedantic -Wno-unused-result -fPIC)
endif()

# Building benchmarks with project
if(BUILD_BENCHMARKS)
  add_subdirectory(benchmarks)
endif()

# Installing eBPF sources in directory with program
set(BPF_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/bpf")
install(
  FILES ${BPF_SOURCE_DIR}/ethernet-parse.c
  DESTINATION ${CMAKE_BINARY_DIR}/bpf
)
