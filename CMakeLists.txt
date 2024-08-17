cmake_minimum_required(VERSION 3.29)
project(chilletcomplier)

set(CMAKE_CXX_STANDARD 20)

add_compile_options(-fmodules-ts)

include_directories(
        src/include src/include/utils src/include/frontend src/include/pass src/include/optimizer src/include/codegen src/include/ir
        src/lib src/frontend/parser)

file(GLOB_RECURSE SRC "src/*.cc" "src/*.hpp")

add_executable(chilletcomplier ${SRC})
