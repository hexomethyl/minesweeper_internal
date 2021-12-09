cmake_minimum_required(VERSION 3.19)
project(minesweeper_internal C)

set(CMAKE_C_STANDARD 99)

add_library(minesweeper_internal SHARED src/dllmain.c src/hook.c src/hook.h)

include_directories("C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)/Include")

find_library(WDK_LIBRARY d3d9 HINTS "C:/Program Files (x86)/Windows Kits/10/Lib/10.0.22000.0/um/64")
find_library(DX_LIBRARY d3dx9 HINTS "C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)/Lib/x64")

target_link_libraries(minesweeper_internal ${WDK_LIBRARY} ${DX_LIBRARY})

