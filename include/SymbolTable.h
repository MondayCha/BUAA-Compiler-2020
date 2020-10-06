//
// Created by Dlee on 2020/10/5.
//

#ifndef SCANNER_SYMBOLTABLE_H
#define SCANNER_SYMBOLTABLE_H

#include <iostream>
#include <string>
#include <map>

using namespace std;

class SymbolTable {
private:
    explicit SymbolTable();

    friend class GrammarAnalyzer;

    map<string, bool> functionType;
public:
    void addFunc(const string &name, bool type);

    bool checkFunc(const string &name);

    static SymbolTable &getInstance();
};

#endif //SCANNER_SYMBOLTABLE_H
