//
// Created by Dlee on 2020/10/5.
//

#ifndef SCANNER_SYMBOLTABLE_H
#define SCANNER_SYMBOLTABLE_H

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include "typeList.h"

using namespace std;

class FuncSym;

class VarSym;

class ConSym;

class Symbol;

class SymbolTable {
private:
    explicit SymbolTable();

    friend class GrammarAnalyzer;

    map<string, FuncSym> globalFuncTable;
    map<string, Symbol> globalIdenTable;
    map<string, Symbol> localIdenTable;
public:
    static SymbolTable &getInstance();

    bool hasIdenName(string &name);

    bool hasIdenNameIncludeGlobal(string &name);

    bool hasFuncName(string &name);

    void insertSymbolToLocal(const Symbol &symbol);

    void insertFuncToGlobal(const FuncSym &funcSym);

    void endGlobalIdenSymbol();

    void endLocalIdenSymbol();

    bool isFuncHasReturn(string &name);

    SymbolType convertTypeCode(TypeCode typeCode);

    SymbolType getFuncType(string &lower_name);

    SymbolType getIdenType(string &lower_name);

    SymbolAtt getIdenAtt(string &lower_name);

    vector<VarSym> getFuncParams(string &lower_name);
};


class Symbol {
private:
    string name;
    string lowerName;
    SymbolAtt symbolAtt;
    SymbolType symbolType;

    friend class SymbolTable;

    friend class GrammarAnalyzer;

public:
    Symbol(string &pronName, string &pronLowerName, SymbolAtt pronAtt, SymbolType pronType);
};

class VarSym : public Symbol {
private:
    int level;
    int length1;
    int length2;

    friend class SymbolTable;

    friend class GrammarAnalyzer;

public:
    VarSym(string &pronName, string &pronLowerName, SymbolType pronType, int plevel, int plength1, int plength2);
};

class FuncSym : public Symbol {
private:
    vector<VarSym> parameters;

    friend class SymbolTable;

    friend class GrammarAnalyzer;

public:
    FuncSym(string &pronName, string &pronLowerName, SymbolType pronType);
};

class ConSym : public Symbol {
private:


    int content;

    friend class SymbolTable;

    friend class GrammarAnalyzer;

public:
    ConSym(string &pronName, string &pronLowerName, SymbolType pronType, int pronContent);
};

#endif //SCANNER_SYMBOLTABLE_H
