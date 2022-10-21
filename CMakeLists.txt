cmake_minimum_required(VERSION 3.5)
project(bpf_test LANGUAGES CXX)

# Initial project settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
set(ENABLE_CPP_API ON)

# Finding libbpf
find_package(bpf REQUIRED)
if (NOT LIBBPF_FOUND)
    message(FATAL "Not found libbpf, please install it")
else()
    message(STATUS "libbpf was found:")
    message(STATUS "Include path: ${LIBBPF_INCLUDE_DIR}")
    message(STATUS "Dynamic lib path: ${LIBBPF_LIBRARIES}")
    message(STATUS "Static lib path: ${LIBBPF_STATIC_LIBRARIES}")
endif()

# Finding libbcc
find_package(bcc REQUIRED)
if (NOT LIBBCC_FOUND)
    message(FATAL "Not found libbcc, please install it")
else()
    message(STATUS "libbcc was found:")
    message(STATUS "Include path: ${LIBCC_INCLUDE_DIR}")
    message(STATUS "Dynamic lib path: ${LIBBCC_LIBRARIES}")
    message(STATUS "Static lib path: ${LIBBCC_STATIC_LIBRARIES}")
endif()

# Finding kernel headers
find_package(kernel-headers REQUIRED)

# Collecting sources of project
file(GLOB_RECURSE files_h ${PROJECT_SOURCE_DIR}/*.h)
file(GLOB_RECURSE files_cpp ${PROJECT_SOURCE_DIR}/*.cpp)

include_directories(${LIBBPF_INCLUDE_DIR})
include_directories(${LIBCC_INCLUDE_DIR})
include_directories("${PROJECT_SOURCE_DIR}/include")

add_executable(${PROJECT_NAME} ${files_h} ${files_cpp})
target_link_libraries(${PROJECT_NAME} PUBLIC ${LIBBPF_LIBRARIES} ${LIBBCC_LIBRARIES})

if (CMAKE_CXX_COMPILER_ID MATCHES GNU|Clang)
    add_compile_options(-Wall -Wextra -Wpedantic -Werror -Wno-unused-result -fPIC)
endif()
