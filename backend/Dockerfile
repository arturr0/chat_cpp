FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    libssl-dev \
    zlib1g-dev

WORKDIR /app

# 👇 pobierz uWebSockets
RUN git clone https://github.com/uNetworking/uWebSockets.git

# 👇 skopiuj Twój kod
COPY . .

# 👇 build
RUN cmake -S . -B build && cmake --build build

CMD ["./build/server"]
