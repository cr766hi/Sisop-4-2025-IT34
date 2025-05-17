FROM ubuntu:latest

RUN apt-get update && apt-get install -y \
    fuse \
    libfuse-dev \
    pkg-config \
    gcc \
    make \
    python3 \
    inotify-tools

WORKDIR /app
COPY antink.c /app/
RUN gcc -Wall antink.c -o antink `pkg-config fuse --cflags --libs`

RUN mkdir -p /antink_mount /antink_anon /var/log

CMD ["/app/antink", "/antink_mount"]
