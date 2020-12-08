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

    friend class MipsTranslator;

    map<string, FuncSym *> globalFuncTable;
    map<string, Symbol *> globalIdenTable;
    map<string, Symbol *> localIdenTable;
public:
    static SymbolTable &getInstance();

    bool hasIdenName(string &name);

    bool hasIdenNameIncludeGlobal(string &name);

    bool hasFuncName(string &name);

    void insertSymbolToLocal(Symbol *symbol);

    void insertFuncToGlobal(FuncSym *funcSym);

    void endGlobalIdenSymbol();

    SymbolType convertTypeCode(TypeCode typeCode);

    SymbolType getFuncType(string &lower_name);

    SymbolType getIdenType(string &lower_name);

    SymbolAtt getIdenAtt(string &lower_name);

    Symbol *getSymbolPtr(map<string, Symbol *> *searchMap, string &lower_name, bool &isGlobal);

    vector<VarSym> getFuncParams(string &lower_name);

    FuncSym *getFuncPtr(string &lower_name);

    void printMap(map<string, Symbol *> *searchMap, string mapName);
};

class Symbol {
public:
    string name;
    string lowerName;
    SymbolAtt symbolAtt;
    SymbolType symbolType;

    Symbol(string &pronName, string &pronLowerName, SymbolAtt pronAtt, SymbolType pronType);

    string toString();
};

class VarSym : public Symbol {
public:
    int level;
    int length1;
    int length2;
    int offset;

    VarSym(string &pronName, string &pronLowerName, SymbolType pronType, int plevel, int plength1, int plength2,
           int pronOffset);
};

class FuncSym : public Symbol {
public:
    vector<VarSym> parameters;
    map<string, Symbol *> funcLocalTable;
    int funcSpace;

    FuncSym(string &pronName, string &pronLowerName, SymbolType pronType);

    void setFuncLocalTable(map<string, Symbol *> pronTable);
};

class ConSym : public Symbol {
public:
    int content;

    ConSym(string &pronName, string &pronLowerName, SymbolType pronType, int pronContent);
};

#endif //SCANNER_SYMBOLTABLE_H
