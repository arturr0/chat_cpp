cmake_minimum_required(VERSION 3.10)
project(chat_server)

set(CMAKE_CXX_STANDARD 17)

add_executable(server main.cpp)

# 👇 ręczne linkowanie
target_link_libraries(server pthread)
