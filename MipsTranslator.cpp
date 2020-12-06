//
// Created by dlely on 2020/11/13.
//

#include "MipsTranslator.h"
#include <fstream>
#include <stack>

map<string, Symbol *> *mips_search_map;

MipsTranslator::MipsTranslator(ofstream &mipsFile, IRCodeManager &irCode, SymbolTable &symbolTable)
        : mipsFile(mipsFile), irCode(irCode), symbolTable(symbolTable) {
}

MipsTranslator &MipsTranslator::getInstance(ofstream &mipsFile, IRCodeManager &irCode, SymbolTable &symbolTable) {
    static MipsTranslator initializer(mipsFile, irCode, symbolTable);
    return initializer;
}

void MipsTranslator::translate() {
    // .data
    MIPS_PRINT(".data")
    for (auto s : irCode.getStringDataList()) {
        MIPS_CODE(s.label << ":\t" << ".asciiz\t\"" << s.content << "\"")
    }
    MIPS_CODE("nLine:\t.asciiz\t\"\\n\"")
    // .text
    MIPS_PRINT(".text")
    int value1, value2;
    bool isImm1, isImm2, isGlobal;
    bool hasNotMeetFunc = true;
    FuncSym *mips_p_current_func;
    mips_search_map = &(symbolTable.globalIdenTable);
    list<ThreeAddCode *> codeList = irCode.getThreeAddCodeList();
    stack<ThreeAddCode *> funcParamsStack;
    for (auto thisCode : codeList) {
        irCode.ircodeFile << thisCode->toString() << endl;
        if (true) {
            MIPS_PRINT("############ " << thisCode->toString())
        }
        OperatorType thisOp = thisCode->op;
        isImm1 = false, isImm2 = false, isGlobal = false;
        if (thisOp == OpBEZ || thisOp == OpBNEZ) {
            continue;
        } else if (thisOp == OpFunc){
            if (!hasNotMeetFunc) {
                MIPS_CODE("jr\t$ra")
            }
            hasNotMeetFunc = false;
            mips_p_current_func = symbolTable.getFuncPtr(RS2.str);
            mips_search_map = &(mips_p_current_func->funcLocalTable);
            MIPS_PRINT("f_" << mips_p_current_func->lowerName << ":")
            if (mips_p_current_func->symbolType == MAIN) {
                MIPS_CODE("move\t$fp,\t$sp")
                MIPS_CODE("addiu\t$sp,\t$sp,\t" << to_string(-4 * (mips_p_current_func->funcSpace) - 8))
            }
            mips_search_map = &(mips_p_current_func->funcLocalTable);
        } else if (thisOp == OpScanf) {
            auto *varSym = (VarSym *) symbolTable.getSymbolPtr(mips_search_map,
                                                                 RD1.str, isGlobal);
            if (varSym->symbolType == INT) {
                MIPS_CODE("li\t\t$v0,\t5")
            } else {
                MIPS_CODE("li\t\t$v0,\t12")
            }
            MIPS_CODE("syscall")
            int address = isGlobal ? (varSym->offset * 4) : -(varSym->offset * 4);
            string pp_reg = p_reg;
            MIPS_CODE("sw\t\t$v0,\t" << to_string(address) << "(" << pp_reg << ")")
        } else if (thisOp == OpPrint) {
            SymbolType printType = RD1.type;
            switch (printType) {
                case VOID:
                    // 字符串
                    MIPS_CODE("la\t\t$a0,\t" << (RS2).str)
                    MIPS_CODE("li\t\t$v0,\t4")
                    break;
                default:
                    loadValue(RS2, $a0, true, isImm1, value1);
                    int syscallNum = (printType == INT) ? 1 : 11;
                    MIPS_CODE("li\t\t$v0,\t" << to_string(syscallNum));
            }
            MIPS_CODE("syscall")
        } else if (thisOp == OpASSIGN) {
            loadValue(RS2, $t0, true, isImm1, value1);
            saveValue(RD1.str, $t0);
        } else if (thisOp == OpAssArray) {
            // AssArr  [string  : a]   [string  : tmp_12]      [int/char: 1]
            loadValue(RT3, $t1, true, isImm2, value2);
            loadValue(RS2, $t0, false, isImm1, value1);
            bool isGo = false;
            auto *varSym = (VarSym *) symbolTable.getSymbolPtr(mips_search_map, RD1.str, isGo);
            string pp_reg = (isGo ? reg_gp : reg_fp);
//            int arrayOffset = RS2.num + varSym->offset;
//            int addr = isGo ? (arrayOffset * 4) : -(arrayOffset * 4);
//            MIPS_CODE("sw\t\t$t1,\t" << to_string(addr) << "(" << pp_reg << ")")
            if (isImm1) {
                int addr = isGo ? ((value1 + varSym->offset) * 4) : -((value1 + varSym->offset) * 4);
                MIPS_CODE("sw\t\t$t1,\t" << to_string(addr) << "(" << pp_reg << ")")
            } else {
                int addr = isGo ? (varSym->offset * 4) : -(varSym->offset * 4);
                MIPS_CODE("addiu\t$t2,\t" << pp_reg << ",\t" << to_string(addr))
                MIPS_CODE("sll\t\t$t0,\t$t0,\t2")
                if (isGo) {
                    MIPS_CODE("addu\t\t$t2,\t$t2,\t$t0")
                } else {
                    MIPS_CODE("sub\t\t$t2,\t$t2,\t$t0")
                }
                MIPS_CODE("sw\t\t$t1,\t0($t2)")
            }
        } else if (thisOp == OpGetArray) {
            loadValue(RT3, $t0, false, isImm2, value2);
            auto *varSym = (VarSym *) symbolTable.getSymbolPtr(mips_search_map, RS2.str, isGlobal);
            string pp_reg = (isGlobal ? reg_gp : reg_fp);
            if (isImm2) {
                int arrayOffset = value2 + varSym->offset;
                int addr = isGlobal ? (arrayOffset * 4) : -(arrayOffset * 4);
                MIPS_CODE("lw\t\t$t1,\t" << to_string(addr) << "(" << pp_reg << ")")
            } else {
                int addr = isGlobal ? (varSym->offset * 4) : -(varSym->offset * 4);
                MIPS_CODE("addiu\t$t2,\t" << pp_reg << ",\t" << to_string(addr))
                MIPS_CODE("sll\t\t$t0,\t$t0,\t2")
                if (isGlobal) {
                    MIPS_CODE("addu\t\t$t2,\t$t2,\t$t0")
                } else {
                    MIPS_CODE("sub\t\t$t2,\t$t2,\t$t0")
                }
                MIPS_CODE("lw\t\t$t1,\t0($t2)")
            }
            saveValue(RD1.str, $t1);
        } else if (thisOp == OpPLUS) {
            loadValue(RS2, $t0, false, isImm1, value1);
            loadValue(RT3, $t1, false, isImm2, value2);
            if (isImm1 && isImm2) {
                MIPS_CODE("li\t\t$t2,\t" << to_string(value1 + value2))
            } else if (!isImm1 && isImm2) {
                MIPS_CODE("addiu\t$t2,\t$t0,\t" << to_string(value2))
            } else if (isImm1 && !isImm2) {
                MIPS_CODE("addiu\t$t2,\t$t1,\t" << to_string(value1))
            } else {
                MIPS_CODE("addu\t\t$t2,\t$t0,\t$t1")
            }
            saveValue(RD1.str, $t2);
        } else if (thisOp == OpMINU) {
            loadValue(RS2, $t0, false, isImm1, value1);
            loadValue(RT3, $t1, false, isImm2, value2);
            if (isImm1 && isImm2) {
                MIPS_CODE("li\t\t$t2,\t" << to_string(value1 - value2))
            } else if (!isImm1 && isImm2) {
                MIPS_CODE("addiu\t$t2,\t$t0,\t" << to_string(-value2))
            } else if (isImm1 && !isImm2) {
                MIPS_CODE("sub\t\t$t1,\t$0,\t$t1")
                MIPS_CODE("addiu\t$t2,\t$t1,\t" << to_string(value1))
            } else {
                MIPS_CODE("sub\t\t$t2,\t$t0,\t$t1")
            }
            saveValue(RD1.str, $t2);
        } else if (thisOp == OpMULT) {
            loadValue(RS2, $t0, true, isImm1, value1);
            loadValue(RT3, $t1, true, isImm2, value2);
            MIPS_CODE("mult\t$t0,\t$t1")
            MIPS_CODE("mflo\t$t2")
            saveValue(RD1.str, $t2);
        } else if (thisOp == OpDIV) {
            loadValue(RS2, $t0, true, isImm1, value1);
            loadValue(RT3, $t1, true, isImm2, value2);
            MIPS_CODE("div\t\t$t0,\t$t1")
            MIPS_CODE("mflo\t$t2")
            saveValue(RD1.str, $t2);
        } else if (thisOp == OpExit) {
            MIPS_CODE("li\t\t$v0,\t10")
            MIPS_CODE("syscall")
        } else if (thisOp == OpJMain) {
            MIPS_CODE("j\tf_main")
        } else if (thisOp == OpLabel) {
            MIPS_PRINT(RD1.str << ":")
        } else if (thisOp == OpJmp) {
            MIPS_CODE("j\t\t" << RD1.str)
        } else if (thisOp == OpEQL || thisOp == OpNEQ) {
            loadValue(RS2, $t0, false, isImm1, value1);
            loadValue(RT3, $t1, false, isImm2, value2);
            bool branch_when_false = thisCode->p_next->op == OpBEZ;
            string branch_model = (branch_when_false ^ (thisOp == OpNEQ)) ? "bne\t\t" : "beq\t\t";
            if (isImm1 && isImm2) {
                bool equalValue = branch_when_false ^(value1 == value2);
                if ((thisOp == OpEQL && equalValue)
                    || (thisOp == OpNEQ && !equalValue)) {
                    MIPS_CODE("j\t" << (thisCode->p_next->obj[0].str))
                }
            } else {
                if (!isImm1 && isImm2) {
                    MIPS_CODE("li\t\t$t1,\t" << to_string(value2))
                } else if (isImm1 && !isImm2) {
                    MIPS_CODE("li\t\t$t0,\t" << to_string(value1))
                }
                MIPS_CODE(branch_model << "$t0,\t$t1,\t" << (thisCode->p_next->obj[0].str))
            }
        } else if (thisOp == OpLSS || thisOp == OpLEQ || thisOp == OpGRE || thisOp == OpGEQ) {
            loadValue(RS2, $t0, false, isImm1, value1);
            loadValue(RT3, $t1, false, isImm2, value2);
            bool branch_when_true = (thisCode->p_next->op == OpBNEZ);
            if (thisOp == OpLSS) {
                CONDITION(value1 < value2, "blt", "bgt", "bge", "ble")
            } else if (thisOp == OpLEQ) {
                CONDITION(value1 <= value2, "ble", "bge", "bgt", "blt")
            } else if (thisOp == OpGRE) {
                CONDITION(value1 > value2, "bgt", "blt", "ble", "bge")
            } else {
                CONDITION(value1 >= value2, "bge", "ble", "blt", "bgt")
            }
        } else if (thisOp == OpReturn) {
            if (RD1.type != VOID) {
                loadValue(RS2, $v0, true, isImm1, value1);
            }
            MIPS_CODE("jr\t\t$ra")
        } else if (thisOp == OpRetVar) {
            int retOffset = ((VarSym *) symbolTable.getSymbolPtr(mips_search_map, RS2.str, isGlobal))->offset;
            MIPS_CODE("sw\t\t$v0,\t" << to_string(-4 * retOffset) << "($fp)")
        } else if (thisOp == OpPaVal) {
            funcParamsStack.push(thisCode);
        } else if (thisOp == OpCall) {
            ThreeAddCode *paramsPtr;
            FuncSym *callFuncPtr = symbolTable.getFuncPtr(RD1.str);
            int callFuncParamsSize = callFuncPtr->parameters.size();
            while (callFuncParamsSize > 0) {
                paramsPtr = funcParamsStack.top();
                funcParamsStack.pop();
                loadValue(paramsPtr->obj[1], $t0, true, isImm1, value1);
                callFuncParamsSize--;
                MIPS_CODE("sw\t\t$t0,\t" << to_string(-4 * callFuncParamsSize) << "($sp)")
            }
            int funcOffset = 4 * callFuncPtr->funcSpace + 8;
            MIPS_CODE("addiu\t$sp,\t$sp,\t" << to_string(-funcOffset))
            MIPS_CODE("sw\t\t$ra,\t4($sp)")
            MIPS_CODE("sw\t\t$fp,\t8($sp)")
            MIPS_CODE("addiu\t$fp,\t$sp,\t" << to_string(funcOffset))
            MIPS_CODE("jal\t\tf_" << RD1.str)
            MIPS_CODE("lw\t\t$fp,\t8($sp)")
            MIPS_CODE("lw\t\t$ra,\t4($sp)")
            MIPS_CODE("addiu\t$sp,\t$sp,\t" << to_string(funcOffset))
        }
    }

}

void MipsTranslator::saveValue(string &obj, const string &regName) {
    bool isGlobal = false;
    auto *varSym = (VarSym *) symbolTable.getSymbolPtr(mips_search_map, obj, isGlobal);
    int address = isGlobal ? (varSym->offset * 4) : -(varSym->offset * 4);
    string pp_reg = p_reg;
    MIPS_CODE("sw\t\t" << regName << ",\t" << to_string(address) << "(" << pp_reg << ")")
}

void MipsTranslator::loadValue(GoalObject &goalObject, const string &regName, bool ifGen, bool &isImm, int &value) {
    bool isGlobal = false;
    Symbol *p_symbol = symbolTable.getSymbolPtr(mips_search_map, goalObject.str, isGlobal);
    if (p_symbol != nullptr) {
        if (p_symbol->symbolAtt == CON) {
            if (ifGen) {
                MIPS_CODE("li\t\t" << regName << ",\t" << to_string(((ConSym *) p_symbol)->content));
            }
            value = ((ConSym *) p_symbol)->content;
            isImm = true;
        } else {
            int address = isGlobal ? (((VarSym *) p_symbol)->offset * 4) : -(((VarSym *) p_symbol)->offset * 4);
            string pp_reg = p_reg;
            MIPS_CODE("lw\t\t" << regName << ",\t" << to_string(address) << "(" << pp_reg << ")")
        }
    } else {
        if (goalObject.branch == 2) {
            if (ifGen) {
                MIPS_CODE("li\t\t" << regName << ",\t" << goalObject.str)
            }
            value = goalObject.num;
            isImm = true;
        } else if (goalObject.str.length() > 0) {
            if (ifGen) {
                MIPS_CODE("li\t\t" << regName << ",\t" << goalObject.str)
            }
            value = 0;
            if (isdigit(goalObject.str[0])) {
                for (char i : goalObject.str) {
                    value = 10 * value + (i - '0');
                }
            } else {
                value = (int) ((unsigned int) goalObject.str[0]);
            }
            isImm = true;
        }
    }
}
