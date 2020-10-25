cmake_minimum_required(VERSION 3.17)
project(Scanner)

set(CMAKE_CXX_STANDARD 11)
#include_directories(SYSTEM "/foo/bar")
add_executable(Scanner main.cpp include/typeList.h Lexer.cpp include/Lexer.h GrammarAnalyzer.cpp include/GrammarAnalyzer.h SymbolTable.cpp include/SymbolTable.h ErrorHandle.cpp include/ErrorHandle.h)