find_path(LIBBPF_INCLUDE_DIR
  NAMES
    bpf/bpf.h
    bpf/btf.h
    bpf/libbpf.h
  PATHS
    /usr/include
    /usr/local/include
    /opt/local/include
    /sw/include
    ENV CPATH)

find_library(LIBBPF_LIBRARIES
  NAMES
    bpf
  PATHS
    /usr/lib
    /usr/local/lib
    /opt/local/lib
    /sw/lib
    ENV LIBRARY_PATH
    ENV LD_LIBRARY_PATH)

if(LIBBPF_LIBRARIES)
    list(APPEND PATHS LIBBPF_LIBRARIES)
endif()

find_library(LIBBPF_STATIC_LIBRARIES
  NAMES
    libbpf.a
  PATHS
    /usr/lib
    /usr/local/lib
    /opt/local/lib
    /sw/lib
    ENV LIBRARY_PATH
    ENV LD_LIBRARY_PATH)
if(LIBBPF_STATIC_LIBRARIES)
    list(APPEND PATHS LIBBPF_STATIC_LIBRARIES)
endif()

if((LIBBPF_INCLUDE_DIR) AND (LIBBPF_LIBRARIES AND LIBBPF_STATIC_LIBRARIES))
    set(LIBBPF_FOUND 1)
else()
    set(LIBBPF_FOUND 0)
endif()

mark_as_advanced(LIBBPF_FOUND)
