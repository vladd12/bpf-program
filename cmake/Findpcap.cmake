find_path(LIBPCAP_INCLUDE_DIR
  NAMES
    pcap.h
    pcap-bpf.h
  PATHS
    /usr/include
    /usr/local/include
    /opt/local/include
    /sw/include
    ENV CPATH)

find_library(LIBPCAP_LIBRARIES
  NAMES
    pcap
  PATHS
    /usr/lib
    /usr/local/lib
    /opt/local/lib
    /sw/lib
    ENV LIBRARY_PATH
    ENV LD_LIBRARY_PATH)

if(LIBPCAP_LIBRARIES)
    list(APPEND PATHS LIBPCAP_LIBRARIES)
endif()

find_library(LIBPCAP_STATIC_LIBRARIES
  NAMES
    libpcap.a
  PATHS
    /usr/lib
    /usr/local/lib
    /opt/local/lib
    /sw/lib
    ENV LIBRARY_PATH
    ENV LD_LIBRARY_PATH)

if(LIBPCAP_STATIC_LIBRARIES)
    list(APPEND PATHS LIBPCAP_STATIC_LIBRARIES)
endif()

if(LIBPCAP_INCLUDE_DIR AND LIBPCAP_LIBRARIES)
    set(LIBPCAP_FOUND 1)
else()
    set(LIBPCAP_FOUND 0)
endif()

mark_as_advanced(LIBPCAP_FOUND)
