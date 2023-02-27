# Docker image with debian x86_64 for cross compiling linux kernel arm64
docker build --file "linux-kernel.dockerfile" --platform linux/amd64 -t lkrt-arm64 .
docker run --platform=linux/amd64 -i -t lkrt-arm64

# Docker image with debian x86_64 for cross compiling app arm64
docker build --file "Dockerfile" --platform linux/amd64 -t bpf-arm64 .
docker run --platform=linux/amd64 -i -t bpf-arm64
