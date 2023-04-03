# Docker image with debian x86_64 for cross compile armhf
docker build --file "Dockerfile" --platform linux/amd64 -t cross_amd64 .
docker run --platform=linux/amd64 -i -t cross_amd64

# Docker image with debian x86 for cross compiling lkrt to armhf
#docker build --file "Dockerfile.LinuxKernel" --platform linux/amd64 -t lkrt-cross-armhf .
#docker run --platform=linux/amd64 -i -t lkrt-cross-armhf

# Getting Docker container ID
docker ps 
cd C:/Temp/files
docker cp <ID>:/home/user/bpf-program/build/bpf_test bpf_test
docker cp <ID>:/home/user/bpf-program/build/bpf .
docker cp <ID>:/usr/lib/libbcc.a libbcc.a
docker cp <ID>:/usr/lib/libbcc.so.0.25.0 libbcc.so.0.25.0
ln -s ./libbcc.so.0.25.0 ./libbcc.so.0
ln -s ./libbcc.so.0 ./libbcc.so

sudo mkdir /usr/src/linux-headers-$(uname -r)
cd /usr/src/linux-headers-$(uname -r)
sudo tar -xvf /sys/kernel/kheaders.tar.xz
