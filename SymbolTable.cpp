//
// Created by Dlee on 2020/10/5.
//

#include "include/SymbolTable.h"

SymbolTable::SymbolTable() = default;

SymbolTable &SymbolTable::getInstance() {
    static SymbolTable instance;
    return instance;
}

void SymbolTable::addFunc(const string &name, bool type) {
    functionType.insert({name, type});
}

bool SymbolTable::checkFunc(const string &name) {
    auto iter = functionType.find(name);
    if (iter != functionType.end()) {
        return iter->second;
    }
    return false;
}
