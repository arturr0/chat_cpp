# Wybieramy obraz bazowy
FROM ubuntu:22.04

# Instalacja zależności
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

# Kopiujemy frontend do folderu /app/frontend
COPY ../frontend ./frontend

# Build serwera
RUN cmake -S . -B build && cmake --build build

# Uruchomienie serwera
CMD ["./build/server"]
