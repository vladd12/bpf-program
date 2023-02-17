# Docker image with debian x86_64 for cross compile armhf
docker build --file "Dockerfile" --platform linux/amd64 -t cross_amd64 .
docker run --platform=linux/amd64 -i -t cross_amd64

# Getting Docker container ID
docker ps 
cd C:/Temp/files
docker cp <ID>:/home/user/bpf-program/build/bpf_test bpf_test
docker cp <ID>:/home/user/bpf-program/build/bpf .

#docker cp <ID>:/usr/lib/arm-linux-gnueabihf/libbpf.a libbpf.a
#docker cp <ID>:/usr/lib/arm-linux-gnueabihf/libbpf.so.0.3.0 libbpf.so.0.3.0
#ln -s ./libbpf.so.0.3.0 ./libbpf.so.0
#ln -s ./libbpf.so.0.3.0 ./libbpf.so

docker cp <ID>:/usr/lib/libbcc-loader-static.a libbcc-loader-static.a
docker cp <ID>:/usr/lib/libbcc.a libbcc.a
docker cp <ID>:/usr/lib/libbcc.so.0.17.0 libbcc.so.0.17.0
ln -s ./libbcc.so.0.25.0 ./libbcc.so.0
ln -s ./libbcc.so.0.25.0 ./libbcc.so

docker cp <ID>:/usr/lib/libbcc_bpf.a libbcc_bpf.a
docker cp <ID>:/usr/lib/libbcc_bpf.so.0.17.0 libbcc_bpf.so.0.17.0
ln -s ./libbcc_bpf.so.0.17.0 ./libbcc_bpf.so.0
ln -s ./libbcc_bpf.so.0.17.0 ./libbcc_bpf.so

sudo mkdir /usr/src/linux-headers-$(uname -r)
cd /usr/src/linux-headers-$(uname -r)
sudo tar -xvf /sys/kernel/kheaders.tar.xz
