//
// Created by dlely on 2020/12/8.
//

#include "ActiveOptimize.h"
#include <set>

ActiveOptimize::ActiveOptimize(SymbolTable &pronSymbol, IRCodeManager &pronCode) : symbolTable(pronSymbol),
                                                                                   irCode(pronCode) {
    mainPtr = nullptr;
}

ActiveOptimize &ActiveOptimize::getInstance(SymbolTable &pronSymbol, IRCodeManager &pronCode) {
    static ActiveOptimize instance(pronSymbol, pronCode);
    return instance;
}

void ActiveOptimize::optimizeCode() {
    splitBasicBlock();
}

void ActiveOptimize::splitBasicBlock() {
    queue<FuncSym *> usedFuncQueue;
    usedFuncQueue.push(mainPtr);
    FuncSym *func;
    while (!usedFuncQueue.empty()) {
        func = usedFuncQueue.front();
        usedFuncQueue.pop();
        for (ThreeAddCode *code : func->funcIRCodeList) {
            FuncSym *use_func_ptr;
            OperatorType cop = code->op;
            if (cop == OpCall) {
                if (!usedFuncMapHas(code->obj[0].str)) {
                    use_func_ptr = symbolTable.getFuncPtr(code->obj[0].str);
                    usedFuncQueue.push(use_func_ptr);
                    usedFuncMap.insert({code->obj[0].str, use_func_ptr});
                }
            } else if (cop == OpPrint) {
                if (code->obj[0].type != VOID && !code->obj[1].str.empty() && code->obj[1].branch == 0) {
                    func->addCountOfVar(code->obj[1].str);
                }
            } else if (cop == OpBNEZ || cop == OpBEZ) {
                if (!code->obj[1].str.empty() && code->obj[1].branch == 0) {
                    func->addCountOfVar(code->obj[1].str);
                }
            } else if (cop == OpPLUS || cop == OpMINU || cop == OpMULT || cop == OpDIV || cop == OpASSIGN ||
                       cop == OpScanf || cop == OpLSS || cop == OpLEQ ||
                       cop == OpGRE || cop == OpGEQ || cop == OpEQL || cop == OpNEQ || cop == OpParam ||
                       cop == OpPaVal || cop == OpRetVar || cop == OpReturn) {
                for (auto &i : code->obj) {
                    if (!i.str.empty() && i.branch == 0) {
                        func->addCountOfVar(i.str);
                    }
                }
            } else if (cop == OpAssArray) {
                if (!code->obj[2].str.empty() && code->obj[2].branch == 0) {
                    func->addCountOfVar(code->obj[2].str);
                }
            } else if (cop == OpGetArray) {
                if (!code->obj[0].str.empty() && code->obj[0].branch == 0) {
                    func->addCountOfVar(code->obj[0].str);
                }
            }
        }
        cout << func->lowerName << "\n";
        auto iter = func->varUseCountMap.begin();
        while (iter != func->varUseCountMap.end()) {
            cout << "\t" << iter->first << " " << iter->second << "\n";
            funcVar a(iter->second, iter->first);
            func->varUseCountQueue.push(a);
            iter++;
        }
//        while (!func->varUseCountQueue.empty()){
//            cout << func->varUseCountQueue.top().name << "\n";
//            func->varUseCountQueue.pop();
//        }
    }
//    auto itera = usedFuncMap.begin();
//    while (itera != usedFuncMap.end()) {
//        cout << itera->first << "\n";
//        itera++;
//    }
}

bool ActiveOptimize::usedFuncMapHas(string &lowerName) {
    auto iter = usedFuncMap.find(lowerName);
    if (iter != usedFuncMap.end()) {
        // cout << "has iden name " << iter->second.name << " add is " << &(iter->second.name) << endl;
        return true;
    }
    return false;
}
