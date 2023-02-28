find_path(LIBBCC_INCLUDE_DIR
  NAMES
    bcc/bcc_common.h
    bcc/bcc_elf.h
    bcc/bcc_exception.h
    bcc/bcc_proc.h
    bcc/bcc_syms.h
    bcc/bcc_usdt.h
    bcc/bcc_version.h
    bcc/bpf_module.h
    bcc/BPF.h
    bcc/BPFTable.h
    bcc/file_desc.h
    bcc/libbpf.h
    bcc/perf_reader.h
    bcc/table_desc.h
    bcc/table_storage.h
  PATHS
    /usr/include
    /usr/local/include
    /opt/local/include
    /sw/include
    ENV CPATH)

find_library(LIBBCC_LIBRARIES
  NAMES
    bcc
  PATHS
    /usr/lib
    /usr/local/lib
    /opt/local/lib
    /sw/lib
    ENV LIBRARY_PATH
    ENV LD_LIBRARY_PATH)

if(LIBBCC_LIBRARIES)
    list(APPEND PATHS LIBBCC_LIBRARIES)
endif()

find_library(LIBBCC_STATIC_LIBRARIES
  NAMES
    libbcc.a
  PATHS
    /usr/lib
    /usr/local/lib
    /opt/local/lib
    /sw/lib
    ENV LIBRARY_PATH
    ENV LD_LIBRARY_PATH)

if(LIBBCC_STATIC_LIBRARIES)
    list(APPEND PATHS LIBBCC_STATIC_LIBRARIES)
endif()

if(LIBBCC_INCLUDE_DIR AND LIBBCC_LIBRARIES)
    set(LIBBCC_FOUND 1)
else()
    set(LIBBCC_FOUND 0)
endif()

mark_as_advanced(LIBBCC_FOUND)
