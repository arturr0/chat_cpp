# backend/Dockerfile
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    libboost-all-dev \
    wget \
    curl \
    nano \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Kopiujemy backend
COPY . .

# Kopiujemy frontend z katalogu nadrzędnego
COPY ../frontend ./frontend

# Build
RUN cmake -S . -B build && cmake --build build

CMD ["./build/server"]
