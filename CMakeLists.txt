cmake_minimum_required(VERSION 3.23)
project(AboutUsUni C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(AboutUsUni
        hashmap.c
        hashmap.h main.c dynamic.c dynamic.h)
