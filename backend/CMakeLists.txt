cmake_minimum_required(VERSION 3.10)
project(chat_server)

set(CMAKE_CXX_STANDARD 17)

find_package(Boost REQUIRED COMPONENTS system filesystem thread)

add_executable(server main.cpp)
target_link_libraries(server Boost::system Boost::filesystem Boost::thread pthread)
