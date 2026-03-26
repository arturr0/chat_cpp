FROM ubuntu:22.04

# 🔧 potrzebne narzędzia i boost
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
COPY ./backend /app

# Kopiujemy frontend
COPY ./frontend /app/frontend

# Build
RUN cmake -S . -B build && cmake --build build

# Uruchomienie
CMD ["./build/server"]
