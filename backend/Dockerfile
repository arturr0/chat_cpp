FROM ubuntu:22.04

# Build tools
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    libssl-dev \
    libboost-all-dev \
    wget \
    curl \
    nano \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Pobieramy uWebSockets v20
RUN git clone -b v20.4.0 --single-branch https://github.com/uNetworking/uWebSockets.git /uwebsockets && \
    cd /uwebsockets && mkdir build && cd build && cmake .. && make install

# Kopiujemy backend i frontend (frontend w tym samym folderze)
COPY . .

# Build projektu
RUN cmake -S . -B build && cmake --build build

# Uruchomienie serwera
CMD ["./build/server"]
