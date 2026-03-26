FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    libssl-dev

WORKDIR /app

COPY . .

RUN cmake . && make

CMD ["./server"]