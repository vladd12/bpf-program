# BPF testing project

## Installing compile instruments
```
sudo apt install -y --no-install-recommends git cmake ninja-build gcc build-essential
```

## Installing dependencies
```
apt search linux-headers-$(uname -r)
sudo apt install linux-headers-$(your_kernel)
sudo apt install libbpfcc-dev xz-utils libpcap-dev
```

## Building on local machine
```
mkdir build && cd build
cmake -G Ninja ..
cmake --build . && cmake --install .
```

## Building with Docker
### arm64
```
mkdir build && cd build
docker build --file "Dockerfile" --platform linux/amd64 -t bpf-arm64 --no-cache ../tools/docker-cross-arm64
docker run --platform=linux/amd64 -i -t bpf-arm64
```
