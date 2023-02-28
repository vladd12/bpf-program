# Pull official Debian bullseye image
FROM debian:bullseye-slim

# Environment variable for
ENV DEBIAN_FRONTEND=noninteractive

# Install instruments
RUN apt update && apt full-upgrade -y && apt install -y --no-install-recommends \
    gcc cmake ninja-build build-essential python3-distutils \
    locales sudo ca-certificates file \
    git curl nano tar pkg-config wget && \
    apt-get -qq clean && \
    rm -rf /var/lib/apt/lists/* 

# Reconfigure locale
RUN locale-gen en_US.UTF-8 && dpkg-reconfigure locales

# Adding new target compile architecture
RUN dpkg --add-architecture arm64 && apt update && \
    apt install -y build-essential \
    pkg-config dpkg-cross \
    binutils:amd64 \
    libstdc++-10-dev:arm64 \
    gcc-aarch64-linux-gnu \
    binutils-aarch64-linux-gnu \
    cross-config \
    crossbuild-essential-arm64 && \
    apt-get -qq clean && \
    rm -rf /var/lib/apt/lists/*

# Installing libbcc dependecies and Linux kernel headers
RUN echo "deb [arch=arm64,amd64] http://ftp.ru.debian.org/debian bullseye main" >> /etc/apt/sources.list && \
    apt update && apt install -y --no-install-recommends linux-libc-dev:arm64 \
    libclang-dev:arm64 libclang-cpp-dev:arm64 llvm-dev:arm64 \
    libelf-dev:arm64 zlib1g-dev:arm64 libbpfcc-dev:arm64 && \
    mkdir /tmp/work

# Add group and user + sudo
RUN groupadd -r user && useradd --create-home --gid user user && echo 'user ALL=NOPASSWD: ALL' > /etc/sudoers.d/user
USER user
ENV HOME /home/user
VOLUME /home/user
WORKDIR /home/user

# Copying cross-compiler and unpacking archive
COPY ./cross_compiler.tgz /home/user/cross_compiler.tgz
RUN tar -xzf cross_compiler.tgz && \
    sudo cp -a ./opt/FriendlyARM/toolchain/11.3-aarch64 ./aarch64-cortexa53 && \
    sudo rm -rf ./opt ./cross_compiler.tgz

# Set C/C++ compilers in environment
ENV CC=/home/user/aarch64-cortexa53/bin/aarch64-cortexa53-linux-gnu-gcc
ENV CXX=/home/user/aarch64-cortexa53/bin/aarch64-cortexa53-linux-gnu-g++

# Copying toolchain file in the image
COPY ./aarch64-cortexa53-toolchain.cmake /home/user/aarch64-cortexa53-toolchain.cmake
ENV TOOLCHAIN_PATH=/home/user/aarch64-cortexa53-toolchain.cmake
ENV LIBRARY_PATH=/usr/lib/aarch64-linux-gnu

# Cloning working repository from GitHub
RUN git clone https://github.com/vladd12/bpf-program.git && cd bpf-program && \
    git switch develop && \
    mkdir build && cd ..

# Setting working directory for application build
WORKDIR /home/user/bpf-program/build

# Build main application from sources for armhf arch
#RUN cmake -G Ninja -DCMAKE_BUILD_TYPE=Release \
#    -DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN_PATH .. && \
#    cmake --build . --config Release && \
#    cmake --install .

CMD ["/bin/sh"]
