cmake_minimum_required(VERSION 3.26)
project(hw4_os C)

set(CMAKE_C_STANDARD 11)

add_executable(hw4_os sync.atlibby.akirscht.c
        simple-mutex-example.c)
