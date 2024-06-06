cmake_minimum_required(VERSION 3.27)
project("TLS Socket" C)

include_directories(./src)
add_executable(main ./src/main.c)

