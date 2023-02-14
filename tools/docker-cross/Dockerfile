# Pull official Debian bullseye image
FROM debian:bullseye-slim

# Environment variable for
ENV DEBIAN_FRONTEND=noninteractive

# Install instruments
RUN apt update && apt full-upgrade -y && apt install -y --no-install-recommends \
    gcc cmake ninja-build build-essential python3-distutils \
    locales sudo ca-certificates file \
    git curl nano tar pkg-config \
    python bison flex && \
    apt-get -qq clean && \
    rm -rf /var/lib/apt/lists/* 

# Reconfigure locale
RUN locale-gen en_US.UTF-8 && dpkg-reconfigure locales

# Adding new target compile architecture
RUN dpkg --add-architecture armhf && apt update && \
    apt install -y build-essential \
    pkg-config dpkg-cross \
    binutils:amd64 \
    libstdc++-10-dev:armhf \
    gcc-arm-linux-gnueabihf \
    cross-config \
    crossbuild-essential-armhf && \
    apt-get -qq clean && \
    rm -rf /var/lib/apt/lists/*

# Installing libbcc dependecies and Linux kernel headers
RUN echo "deb [arch=armhf,amd64] http://ftp.ru.debian.org/debian bullseye main" >> /etc/apt/sources.list && \
    apt update && apt install -y --no-install-recommends linux-libc-dev:armhf \
    libclang-dev:armhf libclang-cpp-dev:armhf llvm-dev:armhf \
    libelf-dev:armhf zlib1g-dev:armhf libfl-dev:armhf && \
    mkdir /tmp/work

# Add group and user + sudo
RUN groupadd -r user && useradd --create-home --gid user user && echo 'user ALL=NOPASSWD: ALL' > /etc/sudoers.d/user
USER user
ENV HOME /home/user
VOLUME /home/user
WORKDIR /home/user

# Set C/C++ compilers in environment
ENV CC=/usr/bin/arm-linux-gnueabihf-gcc
ENV CXX=/usr/bin/arm-linux-gnueabihf-g++
ENV LIBRARY_PATH=/usr/lib/arm-linux-gnueabihf

# Copying toolchain file in the image
COPY ./armhf-gcc-toolchain.cmake /home/user/armhf-gcc-toolchain.cmake
ENV TOOLCHAIN_PATH=/home/user/armhf-gcc-toolchain.cmake

# Copying kernel headers in the image
COPY ./kheaders.tar.xz /home/user/kheaders.tar.xz
RUN sudo mkdir /usr/src/linux-headers && cd /usr/src/linux-headers && \
    sudo tar -xvf /home/user/kheaders.tar.xz
ENV KERNEL_HEADERS_ROOT=/usr/src/linux-headers
ENV KERNEL_HEADERS_ARCH=$KERNEL_HEADERS_ROOT/arch/arm/include
ENV KERNEL_HEADERS_MAIN=$KERNEL_HEADERS_ROOT/include

# Cloning libbpf repo from GitHub, building version 0.3
RUN git clone https://github.com/libbpf/libbpf.git && cd libbpf && \
    git checkout -b stable 051a400 && cd src && mkdir build root

# Setting working directory for libbpf build
WORKDIR /home/user/libbpf/src

# Building and installing libbpf from sources for armhf arch
RUN PKG_CONFIG_PATH=/usr/lib/arm-linux-gnueabihf/pkgconfig \
    OBJDIR=build DESTDIR=root make install && \
    sudo cp -R root/usr/include/bpf /usr/include && \
    sudo cp -a root/usr/lib64/libbpf.* /usr/lib/arm-linux-gnueabihf/ && \
    sudo cp root/usr/lib64/pkgconfig/libbpf.pc /usr/lib/arm-linux-gnueabihf/pkgconfig
WORKDIR /home/user

# Cloning bcc repo from GitHub, building version 0.17.0 (support kernel version 5.9)
RUN git clone https://github.com/iovisor/bcc.git && cd bcc && \
    git checkout -b stable ad5b82a && \
    mkdir build && cd ..

# Setting working directory for bcc build
WORKDIR /home/user/bcc/build

# Building and installing bcc from sources for armhf arch
RUN cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN_PATH \
    -DKERNEL_INCLUDE_DIRS="$KERNEL_HEADERS_ARCH $KERNEL_HEADERS_MAIN" \
    -DCMAKE_USE_LIBBPF_PACKAGE:BOOL=ON \
    -DENABLE_EXAMPLES:BOOL=OFF -DENABLE_MAN:BOOL=OFF -DENABLE_TESTS:BOOL=OFF .. && \
    cmake --build . --config Release && \
    sudo cmake --install . --config Release
WORKDIR /home/user

# Cloning working repository from GitHub
#RUN git clone https://github.com/vladd12/bpf-program.git && cd bpf-program && \
#    git switch main && \
#    mkdir build && cd ..

# Setting working directory for application build
#WORKDIR /home/user/bpf-program/build

# Build main application from sources for armhf arch
#RUN cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCOMPAT_DIR=/usr/include/uapi \
#    -DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN_PATH .. && \
#    cmake --build . --config Release && \
#    cmake --install .

CMD ["/bin/sh"]
