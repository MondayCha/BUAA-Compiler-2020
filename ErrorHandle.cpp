//
// Created by Dlee on 2020/10/20.
//

#include "include/ErrorHandle.h"

using namespace std;

ErrorHandle::ErrorHandle(SymbolTable &symbolTable, ofstream &errorFile)
        : symbolTable(symbolTable), errorFile(errorFile) {
}

ErrorHandle &ErrorHandle::getInstance(SymbolTable &symbolTable, ofstream &errorFile) {
    static ErrorHandle initializer(symbolTable, errorFile);
    return initializer;
}

void ErrorHandle::printErrorLine(char errorCode, int &lineNum) {
    errorFile << lineNum << " " << errorCode << endl;
}

void ErrorHandle::printErrorLine(char errorCode, int &lineNum, bool &findError) {
    if (!findError) {
        errorFile << lineNum << " " << errorCode << endl;
        findError = true;
    }
}

bool ErrorHandle::checkIllegalCharReturnNull(char &content, int &lineNum, bool &findError) {
    if (content == '\'') {
        findError = true;
        printErrorLine('a', lineNum);
        return true;
    } else if (isalnum(content) || content == '+' || content == '-' || content == '*' || content == '/' ||
               content == '_') {
        return false;
    } else {
        // illegal symbol appears
        findError = true;
        printErrorLine('a', lineNum);
        return false;
    }
}

bool ErrorHandle::checkDupIdenDefine(string &lowerName, int &lineNum) {
    bool exist = symbolTable.hasIdenName(lowerName);
    if (exist) {
        printErrorLine('b', lineNum);
        cerr << "dup iden ! lowerName is " << lowerName << " in line of " << lineNum << endl;
    }
    return exist;
}

bool ErrorHandle::checkDupFuncDefine(string &lowerName, int &lineNum) {
    bool exist = symbolTable.hasFuncName(lowerName);
    if (exist) {
        printErrorLine('b', lineNum);
    }
    return exist;
}

void ErrorHandle::printErrorOfReturnMatch(int &lineNum) {
    printErrorLine('h', lineNum);
}

void ErrorHandle::printErrorOfNoReturnMatch(int &lineNum) {
    printErrorLine('g', lineNum);
}

bool ErrorHandle::checkUndefFuncCall(string &lowerName, int &lineNum) {
    bool not_exist = !symbolTable.hasFuncName(lowerName);
    if (not_exist) {
        printErrorLine('c', lineNum);
        cerr << "un defined func! lowerName is " << lowerName << " in line of " << lineNum << endl;
    }
    return not_exist;
}

bool ErrorHandle::checkArrayIndexNotInt(SymbolType expType, int &lineNum) {
    if (expType != INT && expType != INTVAR) {
        printErrorLine('i', lineNum);
        return true;
    }
    return false;
}

bool ErrorHandle::checkUndefIdenRefer(string &lowerName, int &lineNum) {
    bool not_exist = !symbolTable.hasIdenNameIncludeGlobal(lowerName);
    if (not_exist) {
        printErrorLine('c', lineNum);
        cerr << "un defined iden! lowerName is " << lowerName << " in line of " << lineNum << endl;
    }
    return not_exist;
}

void ErrorHandle::printErrorOfUnMatchConst(int &lineNum) {
    printErrorLine('o', lineNum);
}

void ErrorHandle::checkChangeConstIden(string &lowerName, int &lineNum) {
    SymbolAtt symbolAtt = symbolTable.getIdenAtt(lowerName);
    if (symbolAtt == CON) {
        printErrorLine('j', lineNum);
    }
}
