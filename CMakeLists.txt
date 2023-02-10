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
    message(STATUS "
    libbpf was found:
    Include path: ${LIBBPF_INCLUDE_DIR}
    Dynamic lib path: ${LIBBPF_LIBRARIES}
    Static lib path: ${LIBBPF_STATIC_LIBRARIES}")
endif()

# Finding libbcc
find_package(bcc REQUIRED)
if (NOT LIBBCC_FOUND)
    message(FATAL "Not found libbcc, please install it")
else() 
    message(STATUS "
    libbcc was found:
    Include path: ${LIBCC_INCLUDE_DIR}
    Dynamic lib path: ${LIBBCC_LIBRARIES}
    Static lib path: ${LIBBCC_STATIC_LIBRARIES}")
endif()

# Finding kernel headers
#find_package(kernel-headers REQUIRED)
#if (NOT KERNELHEADERS_FOUND)
#    message(FATAL "Not found kernel headers, please install it")
#else()
#    message(STATUS "
#    Kernel headers were found:
#    Kernel release: ${KERNEL_RELEASE}
#    Kernel headers path: ${KERNELHEADERS_DIR}
#    Kernel include path: ${KERNELHEADERS_INCLUDE_DIRS}
#    ")
#endif()

# Collecting sources of project
file(GLOB_RECURSE files_h ${PROJECT_SOURCE_DIR}/*.h)
file(GLOB_RECURSE files_cpp ${PROJECT_SOURCE_DIR}/*.cpp)

# Adding paths of headers to include
include_directories(${LIBBPF_INCLUDE_DIR})
include_directories(${LIBCC_INCLUDE_DIR})
#include_directories(${KERNELHEADERS_INCLUDE_DIRS})
include_directories("${PROJECT_SOURCE_DIR}/include")

# Compilation project
add_executable(${PROJECT_NAME} ${files_h} ${files_cpp})

# Linking libraries
target_link_libraries(${PROJECT_NAME} PUBLIC ${LIBBPF_LIBRARIES} ${LIBBCC_LIBRARIES})

# Some compilation flags
if (CMAKE_CXX_COMPILER_ID MATCHES GNU|Clang)
    add_compile_options(-Wall -Wextra -Wpedantic -Werror -Wno-unused-result -fPIC)
endif()

# Installing eBPF sources in directory with program
set(BPF_SOURCE_DIR "${PROJECT_SOURCE_DIR}/bpf")
install(
  FILES
    ${BPF_SOURCE_DIR}/ethernet-parse.c
  DESTINATION
    ${CMAKE_BINARY_DIR}/bpf
)
