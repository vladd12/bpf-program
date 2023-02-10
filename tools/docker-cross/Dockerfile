# Pull official Debian bullseye image
FROM debian:bullseye-slim

# Environment variable for
ENV DEBIAN_FRONTEND=noninteractive

# Install instruments
RUN apt update && apt full-upgrade -y && apt install -y --no-install-recommends \
    cmake nano \
    clang \
    python3-distutils \
    ninja-build build-essential \
    locales sudo ca-certificates file\
    git curl pkg-config && \
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
    apt update && apt install -y --no-install-recommends linux-headers-5.10.0-18-common-rt \
    libbpf-dev:armhf libclang-dev:armhf libclang-cpp-dev:armhf \
    libelf-dev:armhf \
    libfl-dev:armhf \
    linux-libc-dev:armhf llvm-dev:armhf && \
    mkdir /tmp/work

# Add group and user + sudo
RUN groupadd -r user && useradd --create-home --gid user user && echo 'user ALL=NOPASSWD: ALL' > /etc/sudoers.d/user
USER user
WORKDIR /home/user
ENV HOME /home/user

# Set C/C++ compilers in environment
ENV CC=/usr/bin/arm-linux-gnueabihf-gcc
ENV CXX=/usr/bin/arm-linux-gnueabihf-g++

# Cloning bcc repo from GitHub
RUN git clone https://github.com/iovisor/bcc.git && \
    cd bcc && mkdir build && cd ..

# Cloning working repository from GitHub
RUN git clone https://github.com/vladd12/bpf-program.git && \
    cd bpf-program && git switch docker-armhf && mkdir build && cd ..
    #git checkout -B "docker-armhf" "origin/docker-armhf"

# Setting working directory for bcc build
WORKDIR /home/user/bcc/build

# Building and installing bcc from sources for armhf
RUN cmake -G Ninja -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE=/home/user/bpf-program/cmake/armhf-gcc-toolchain.cmake \
    -DENABLE_EXAMPLES:BOOL=OFF -DENABLE_MAN:BOOL=OFF -DENABLE_TESTS:BOOL=OFF .. && \
    cmake --build . --config Release && \
    sudo cmake --install . --config Release

# Setting working directory for application build
WORKDIR /home/user/bpf-program/build

# Build main application from sources
RUN cmake -G Ninja -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE=../cmake/armhf-gcc-toolchain.cmake  .. && \
    cmake --build . --config Release

CMD ["/bin/sh"]
