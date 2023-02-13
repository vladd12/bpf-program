# Docker image with debian x86_64 for cross compile armhf
docker build --file "Dockerfile" --platform linux/amd64 -t cross_amd64 .
docker run --platform=linux/amd64 -i -t cross_amd64
