//
// Created by dlely on 2020/12/8.
//

#ifndef MYCOMPILER_ACTIVEOPTIMIZE_H
#define MYCOMPILER_ACTIVEOPTIMIZE_H

#include "IRCodeManager.h"
#include "SymbolTable.h"

class ActiveOptimize {
private:
    explicit ActiveOptimize();

public:
    map<FuncSym *, vector<ThreeAddCode *>> funcIRCodeVector;

    static ActiveOptimize &getInstance();


};

class Block {
public:

};


#endif //MYCOMPILER_ACTIVEOPTIMIZE_H
