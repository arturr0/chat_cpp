cmake_minimum_required(VERSION 3.10)
project(ChatServer)

set(CMAKE_CXX_STANDARD 17)
find_package(Boost REQUIRED COMPONENTS system thread)
find_package(Threads REQUIRED)

add_executable(server main.cpp)
target_link_libraries(server PRIVATE Boost::system Threads::Threads pthread)
