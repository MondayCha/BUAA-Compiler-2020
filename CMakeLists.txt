cmake_minimum_required(VERSION 3.17)
project(Scanner)

set(CMAKE_CXX_STANDARD 11)
#include_directories(SYSTEM "/foo/bar")
add_executable(Scanner main.cpp lib/typeList.h Lexer.cpp lib/Lexer.h)