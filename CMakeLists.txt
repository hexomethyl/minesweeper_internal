cmake_minimum_required(VERSION 3.19)
project(minesweeper_internal C)

set(CMAKE_C_STANDARD 99)

add_library(minesweeper_internal SHARED src/dllmain.c)