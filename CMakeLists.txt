cmake_minimum_required(VERSION 3.17)
project(MyCompiler)

set(CMAKE_CXX_STANDARD 14)

include_directories(include)

add_executable(MyCompiler
        include/ErrorHandle.h
        include/GrammarAnalyzer.h
        include/Lexer.h
        include/SymbolTable.h
        include/typeList.h
        include/IRCodeManager.h
        ErrorHandle.cpp
        GrammarAnalyzer.cpp
        Lexer.cpp
        main.cpp
        SymbolTable.cpp
        IRCodeManager.cpp MipsTranslator.cpp include/MipsTranslator.h)
