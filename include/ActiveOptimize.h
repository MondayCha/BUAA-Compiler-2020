//
// Created by dlely on 2020/12/8.
//

#ifndef MYCOMPILER_ACTIVEOPTIMIZE_H
#define MYCOMPILER_ACTIVEOPTIMIZE_H

#include "IRCodeManager.h"
#include "SymbolTable.h"
#include <queue>

class ActiveOptimize {
private:
    explicit ActiveOptimize(SymbolTable &pronSymbol,IRCodeManager &pronCode);

    SymbolTable &symbolTable;
    IRCodeManager &irCode;


    void splitBasicBlock();

    bool usedFuncMapHas(string &lowerName);

public:
    FuncSym *mainPtr;
    FuncSym *globalPtr;
    map<string, FuncSym *> usedFuncMap;
    list<FuncSym *> funcList;

    static ActiveOptimize &getInstance(SymbolTable &pronSymbol,IRCodeManager &pronCode);

    void optimizeCode();
};

#endif //MYCOMPILER_ACTIVEOPTIMIZE_H
