cmake_minimum_required(VERSION 3.26)
project(Assignment1 C)

set(CMAKE_C_STANDARD 11)

add_executable(Assignment1 main.c
        loader.c
        loader.h
        decoder.c
        decoder.h)
