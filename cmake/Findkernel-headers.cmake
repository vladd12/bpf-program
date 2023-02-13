# Linux kernel version
execute_process(COMMAND uname -r
    OUTPUT_VARIABLE KERNEL_RELEASE
    OUTPUT_STRIP_TRAILING_WHITESPACE)

set(ENV_KERNEL_HEADERS ENV{KERNEL_HEADERS})
message(STATUS "Env : ${ENV_KERNEL_HEADERS}")

# Find the headers
find_path(KERNELHEADERS_DIR
    include/generated/uapi/linux/version.h
    PATHS
    # RedHat derivatives
    /usr/src/kernels/${KERNEL_RELEASE}
    # Debian derivatives
    /usr/src/linux-headers-${KERNEL_RELEASE}
    # Evironment derivatives
    ${ENV_KERNEL_HEADERS})

if (KERNELHEADERS_DIR)
    set(KERNELHEADERS_INCLUDE_DIRS ${KERNELHEADERS_DIR}/include/generated/uapi)
    set(KERNELHEADERS_FOUND 1)
else ()
    set(KERNELHEADERS_FOUND 0)
endif ()

mark_as_advanced(KERNELHEADERS_FOUND)
