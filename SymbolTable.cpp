//
// Created by Dlee on 2020/10/5.
//

#include "include/SymbolTable.h"

using namespace std;

SymbolTable::SymbolTable() = default;

SymbolTable &SymbolTable::getInstance() {
    static SymbolTable instance;
    return instance;
}

bool SymbolTable::hasIdenName(string &name) {
    auto iter = localIdenTable.find(name);
    if (iter != localIdenTable.end()) {
        // cout << "has iden name " << iter->second.name << " add is " << &(iter->second.name) << endl;
        return true;
    }
    return false;
}

bool SymbolTable::hasIdenNameIncludeGlobal(string &name) {
    auto iter = localIdenTable.find(name);
    if (iter != localIdenTable.end()) {
        // cout << "has iden name " << iter->second.name << " add is " << &(iter->second.name) << endl;
        return true;
    }
    auto iter1 = globalIdenTable.find(name);
    if (iter1 != globalIdenTable.end()) {
        return true;
    }
    return false;
}

void SymbolTable::insertSymbolToLocal(Symbol *symbol) {
    localIdenTable.insert({symbol->lowerName, symbol});
}

void SymbolTable::endGlobalIdenSymbol() {
    globalIdenTable = move(localIdenTable);
}

SymbolType SymbolTable::convertTypeCode(TypeCode typeCode) {
    return (typeCode == CHARTK) ? CHAR : INT;
}

bool SymbolTable::hasFuncName(string &name) {
    auto iter = globalFuncTable.find(name);
    if (iter != globalFuncTable.end()) {
        // cout << "has iden name " << iter->second.name << " add is " << &(iter->second.name) << endl;
        return true;
    }
    auto iter1 = globalIdenTable.find(name);
    if (iter1 != globalIdenTable.end()) {
        return true;
    }
    return false;
}

void SymbolTable::insertFuncToGlobal(FuncSym *funcSym) {
    globalFuncTable.insert({funcSym->lowerName, funcSym});
}

SymbolType SymbolTable::getFuncType(string &lower_name) {
    auto iter = globalFuncTable.find(lower_name);
    if (iter != globalFuncTable.end()) {
        // cout << "has iden lower_name " << iter->second.lower_name << " add is " << &(iter->second.lower_name) << endl;
        return iter->second->symbolType;
    }
    return VOID;
}

SymbolType SymbolTable::getIdenType(string &lower_name) {
    auto iter = localIdenTable.find(lower_name);
    if (iter != localIdenTable.end()) {
        // cout << "has iden name " << iter->second.name << " add is " << &(iter->second.name) << endl;
        return iter->second->symbolType;
    }
    auto iter1 = globalIdenTable.find(lower_name);
    if (iter1 != globalIdenTable.end()) {
        // cerr << "name is " << lower_name << " get iden type is " << iter->second.symbolType << endl;
        return iter1->second->symbolType;
    }
    return VOID;
}

SymbolAtt SymbolTable::getIdenAtt(string &lower_name) {
    auto iter = localIdenTable.find(lower_name);
    if (iter != localIdenTable.end()) {
        // cout << "has iden name " << iter->second.name << " add is " << &(iter->second.name) << endl;
        return iter->second->symbolAtt;
    }
    auto iter1 = globalIdenTable.find(lower_name);
    if (iter1 != globalIdenTable.end()) {
        return iter1->second->symbolAtt;
    }
    return VAR;
}

vector<VarSym> SymbolTable::getFuncParams(string &lower_name) {
    auto iter = globalFuncTable.find(lower_name);
    if (iter != globalFuncTable.end()) {
        // cout << "has iden lower_name " << iter->second.lower_name << " add is " << &(iter->second.lower_name) << endl;
        return iter->second->parameters;
    }
    return vector<VarSym>();
}

FuncSym *SymbolTable::getFuncPtr(string &lower_name) {
    auto iter = globalFuncTable.find(lower_name);
    if (iter != globalFuncTable.end()) {
        // cout << "has iden lower_name " << iter->second.lower_name << " add is " << &(iter->second.lower_name) << endl;
        return iter->second;
    }
    cerr << "undefined func!!!!!!" << endl;
    return iter->second;
}

Symbol *SymbolTable::getSymbolPtr(map<string, Symbol *> *searchMap, string &lower_name, bool &isGlobal) {
    if (searchMap != &globalIdenTable) {
        auto iter = searchMap->find(lower_name);
        if (iter != searchMap->end()) {
            // cout << "has iden name " << iter->second.name << " add is " << &(iter->second.name) << endl;
            auto *p = (iter->second);
            return p;
        }
    }
    auto iter1 = globalIdenTable.find(lower_name);
    if (iter1 != globalIdenTable.end()) {
        //return iter1->second;
        auto *p = (iter1->second);
        isGlobal = true;
        return p;
    }
    return nullptr;
}

void SymbolTable::printMap(map<string, Symbol *> *searchMap, string mapName) {
    // cerr << "////////// print map name is " << mapName << " address is " << to_string(reinterpret_cast<int>(searchMap) << endl;
    for (auto p : *searchMap) {
        cerr << p.second->toString() << endl;
    }
}

Symbol::Symbol(string &pronName, string &pronLowerName, SymbolAtt pronAtt, SymbolType pronType) {
    name = pronName;
    lowerName = pronLowerName;
    symbolAtt = pronAtt;
    symbolType = pronType;
}

string Symbol::toString() {
    return to_string(symbolType) + " " + lowerName;
}

//string Symbol::getName() {
//    return name;
//}
//
//SymbolAtt Symbol::getSymbolAtt() {
//    return symbolAtt;
//}
//
//SymbolType Symbol::getSymbolType() {
//    return symbolType;
//}

VarSym::VarSym(string &pronName, string &pronLowerName, SymbolType pronType,
               int plevel, int plength1, int plength2, int pronOffset)
        : Symbol(pronName, pronLowerName, VAR, pronType) {
    level = plevel;
    length1 = plength1;
    length2 = plength2;
    offset = pronOffset;
}

FuncSym::FuncSym(string &pronName, string &pronLowerName, SymbolType pronType)
        : Symbol(pronName, pronLowerName, FUN, pronType) {
}

void FuncSym::setFuncLocalTable(map<string, Symbol *> pronTable) {
    funcLocalTable = move(pronTable);
}

void FuncSym::addThreeAddCode(ThreeAddCode *code) {
    if (!funcIRCodeList.empty()) {
        funcIRCodeList.back()->setNext(code);
    }
    funcIRCodeList.push_back(code);

}

void FuncSym::updateLastCode(string &ans) {
    ThreeAddCode *p_code = funcIRCodeList.back();
    p_code->setObj(0, ans);
}

void FuncSym::addCountOfVar(string &name) {
    if(name[0] != '@'){
        auto iter = varUseCountMap.find(name);
        if (iter != varUseCountMap.end()) {
            int count = (iter->second) + 1;
            varUseCountMap.erase(name);
            varUseCountMap.insert({name, count});
        } else {
            varUseCountMap.insert({name, 1});
        }
    }
}

ThreeAddCode *FuncSym::getLastCodePtr() {
    return funcIRCodeList.back();
}

ConSym::ConSym(string &pronName, string &pronLowerName, SymbolType pronType, int pronContent)
        : Symbol(
        pronName,
        pronLowerName,
        CON, pronType) {
    content = pronContent;
}