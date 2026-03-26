FROM ubuntu:22.04

# Narzędzia do builda
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

# Pobieramy i instalujemy uWebSockets
RUN git clone https://github.com/uNetworking/uWebSockets.git /uwebsockets && \
    cd /uwebsockets && \
    mkdir build && cd build && \
    cmake .. && make install

# Kopiujemy backend (wraz z frontend w tym samym folderze)
COPY . .

# Build projektu
RUN cmake -S . -B build && cmake --build build

# Uruchomienie serwera
CMD ["./build/server"]
