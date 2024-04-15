cmake_minimum_required(VERSION 3.5)
project(bpf_app LANGUAGES CXX)

# Initial project settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Adding path for cmake scripts
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Setting project build type, if not specified
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Build type" FORCE)
  message(STATUS "CMAKE_BUILD_TYPE is not specified. Using default: ${CMAKE_BUILD_TYPE}")
endif()

# Project options
option(BUILD_BENCHMARKS  "Building benchmarks" OFF)
option(BUILD_BPF_ENGINE  "Building BPF engine" ON)
option(BUILD_PCAP_ENGINE "Building pcap engine" ON)

# Collecting sources of project
set(iec_core_h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/iec_core/pipeline_builder.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/iec_core/algo/processing.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/iec_core/engines/base_engine.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/iec_core/engines/native_engine.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/iec_core/iec/byte_op.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/iec_core/iec/iec_parser.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/iec_core/iec/validator.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/iec_core/handlers/number_crusher.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/iec_core/utils/buffer.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/iec_core/utils/code_formater.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/iec_core/utils/fast_file.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/iec_core/utils/helpers.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/iec_core/utils/meta.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/iec_core/utils/socket.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/iec_core/utils/value_exchange.h
)
set(iec_core_cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/iec_core/algo/processing.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/iec_core/iec/byte_op.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/iec_core/iec/validator.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/iec_core/utils/code_formater.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/iec_core/utils/fast_file.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/iec_core/utils/helpers.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/iec_core/utils/socket.cpp
)
set(bpf_app_cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
)

if(BUILD_BPF_ENGINE)
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

  # Append some headers and sources
  list(APPEND iec_core_h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/iec_core/engines/bpf_engine.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/iec_core/engines/bpf_exec.h
  )
  list(APPEND iec_core_cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/iec_core/engines/bpf_exec.cpp
  )
endif()

if(BUILD_PCAP_ENGINE)
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

  # Append some headers and sources
  list(APPEND iec_core_h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/iec_core/engines/pcap_engine.h
  )
  list(APPEND iec_core_cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/iec_core/engines/pcap_engine.cpp
  )
endif()

# Compilation static library as part of project
add_library(iec_core STATIC ${iec_core_h} ${iec_core_cpp})

# Compilation flags for iec_core library
if(CMAKE_CXX_COMPILER_ID MATCHES GNU|Clang)
  target_compile_options(iec_core PUBLIC -Wall -Wextra -Wpedantic -Wno-unused-result -fPIC)
endif()

# Including directories for library
target_include_directories(iec_core PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include;${LIBBCC_LIBRARIES}>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

# Finding and linking pthread library for UNIX systems
if(UNIX)
  find_package(Threads REQUIRED)
  target_link_libraries(iec_core PRIVATE Threads::Threads)
endif()

# Linking libbcc library
if(BUILD_BPF_ENGINE)
  target_link_libraries(iec_core PUBLIC ${LIBBCC_LIBRARIES})
  target_include_directories(iec_core PUBLIC ${LIBBCC_INCLUDE_DIR})
endif()
# Linking libpcap library
if(BUILD_PCAP_ENGINE)
  target_link_libraries(iec_core PUBLIC ${LIBPCAP_LIBRARIES})
  target_include_directories(iec_core PUBLIC ${LIBPCAP_INCLUDE_DIR})
endif()

# Compilation application
add_executable(${PROJECT_NAME} ${bpf_app_cpp})
target_link_libraries(${PROJECT_NAME} PUBLIC iec_core)
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
