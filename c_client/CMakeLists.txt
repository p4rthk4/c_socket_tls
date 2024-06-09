cmake_minimum_required(VERSION 3.2)
project("TLS Socket Client" C)

include_directories(./)
add_executable(client ./main.c)
target_link_libraries(client crypto ssl)
