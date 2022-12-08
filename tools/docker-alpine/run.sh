# Docker image with Alpine for x86_64
docker build --file "Dockerfile" --platform linux/amd64 . -t alpine_amd64 .
docker run --platform=linux/amd64 -i -t alpine_amd64

# Docker image with Alpine for ARM v8
docker build --file "Dockerfile.aarch64" --platform linux/arm64 -t alpine_aarch64 .
docker run -i -t alpine_aarch64

# Docker image with Alpine for ARM v6
docker build --file "Dockerfile.armhf" --platform linux/armhf -t alpine_armhf .
docker run --platform=linux/armhf --volume=./build:/home/payload/bcc/build -i -t alpine_armhf

