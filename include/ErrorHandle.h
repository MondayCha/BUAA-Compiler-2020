//
// Created by Dlee on 2020/10/20.
//

#ifndef SCANNER_ERRORHANDLE_H
#define SCANNER_ERRORHANDLE_H

#include "SymbolTable.h"
#include <iostream>
#include <string>
#include <fstream>

#define ERROR_PRINT(A, N)

class ErrorHandle {
private:
    explicit ErrorHandle(SymbolTable &symbolTable, ofstream &errorFile);   // Singleton Pattern
    SymbolTable &symbolTable;
    ofstream &errorFile;
public:
    static ErrorHandle &getInstance(SymbolTable &symbolTable, ofstream &errorFile);

    void printErrorLine(char errorCode, int &lineNum);

    void printErrorLine(char errorCode, int &lineNum, bool &findError);

    void printErrorOfReturnMatch(int &lineNum);

    bool checkIllegalCharReturnNull(char &content, int &lineNum, bool &findError);

    bool checkDupIdenDefine(string &lowerName, int &lineNum);

    bool checkDupFuncDefine(string &lowerName, int &lineNum);

    bool checkUndefFuncCall(string &lowerName, int &lineNum);

    bool checkArrayIndexNotInt(SymbolType expType, int &lineNum);

    bool checkUndefIdenRefer(string &lowerName, int &lineNum);

    void printErrorOfNoReturnMatch(int &lineNum);

    void printErrorOfUnMatchConst(int &lineNum);

    void checkChangeConstIden(string &lowerName, int &lineNum);
};


#endif //SCANNER_ERRORHANDLE_H
