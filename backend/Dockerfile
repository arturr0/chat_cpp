FROM ubuntu:22.04

# Instalacja narzędzi do kompilacji i Boost
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

# Kopiujemy cały backend razem z frontend
COPY . .

# Build
RUN cmake -S . -B build && cmake --build build

# Uruchomienie serwera
CMD ["./build/server"]
