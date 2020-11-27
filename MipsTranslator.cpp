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
    bool isImm1, isImm2;
    bool hasNotMeetFunc = true;
    FuncSym *mips_p_current_func;
    mips_search_map = &(symbolTable.globalIdenTable);
    list<ThreeAddCode *> codeList = irCode.getThreeAddCodeList();
    for (auto thisCode : codeList) {
        cout << thisCode->toString() << endl;
        if (true) {
            MIPS_PRINT("############ " << thisCode->toString())
        }
        OperatorType thisOp = thisCode->op;
        isImm1 = false, isImm2 = false;
        if (thisOp == OpBEZ || thisOp == OpBNEZ) {
            continue;
        } else if (thisOp == OpFunc) {
            if (!hasNotMeetFunc) {
                MIPS_CODE("jr\t$ra")
            }
            hasNotMeetFunc = false;
            mips_p_current_func = symbolTable.getFuncPtr(thisCode->obj[1].str);
            mips_search_map = &(mips_p_current_func->funcLocalTable);
            MIPS_PRINT("f_" << mips_p_current_func->name << ":")
            if (mips_p_current_func->symbolType == MAIN) {
                MIPS_CODE("move\t$fp,\t$sp")
                MIPS_CODE("addi\t$sp,\t$sp,\t" << to_string(-4 * (mips_p_current_func->funcSpace) - 8))
            }
            mips_search_map = &(mips_p_current_func->funcLocalTable);
        } else if (thisOp == OpScanf) {
            bool isGlobal = false;
            VarSym *varSym = (VarSym *) symbolTable.getSymbolPtr(mips_search_map,
                                                                 thisCode->obj[0].str, isGlobal);
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
            SymbolType printType = thisCode->obj[0].type;
            switch (printType) {
                case VOID:
                    // 字符串
                    MIPS_CODE("la\t\t$a0,\t" << (thisCode->obj[1]).str)
                    MIPS_CODE("li\t\t$v0,\t4")
                    break;
                default:
                    loadValue(thisCode->obj[1], $a0, true, isImm1, value1);
                    int syscallNum = (printType == INT) ? 1 : 11;
                    MIPS_CODE("li\t\t$v0,\t" << to_string(syscallNum));
            }
            MIPS_CODE("syscall")
        } else if (thisOp == OpASSIGN) {
            loadValue(thisCode->obj[1], $t0, true, isImm1, value1);
            saveValue(thisCode->obj[0].str, $t0);
        } else if (thisOp == OpAssArray) {
            // AssArr  [string  : a]   [string  : tmp_12]      [int/char: 1]
            loadValue(thisCode->obj[2], $t1, true, isImm2, value2);
            loadValue(thisCode->obj[1], $t0, false, isImm1, value1);
            bool isGo = false;
            auto *varSym = (VarSym *) symbolTable.getSymbolPtr(mips_search_map, thisCode->obj[0].str, isGo);
            string pp_reg = (isGo ? reg_gp : reg_fp);
//            int arrayOffset = thisCode->obj[1].num + varSym->offset;
//            int addr = isGo ? (arrayOffset * 4) : -(arrayOffset * 4);
//            MIPS_CODE("sw\t\t$t1,\t" << to_string(addr) << "(" << pp_reg << ")")
            if (isImm1) {
                int addr = isGo ? ((value1 + varSym->offset) * 4) : -((value1 + varSym->offset) * 4);
                MIPS_CODE("sw\t\t$t1,\t" << to_string(addr) << "(" << pp_reg << ")")
            } else {
                int addr = isGo ? (varSym->offset * 4) : -(varSym->offset * 4);
                MIPS_CODE("addi\t$t2,\t" << pp_reg << ",\t" << to_string(addr))
                MIPS_CODE("sll\t\t$t0,\t$t0,\t2")
                if (isGo) {
                    MIPS_CODE("add\t\t$t2,\t$t2,\t$t0")
                } else {
                    MIPS_CODE("sub\t\t$t2,\t$t2,\t$t0")
                }
                MIPS_CODE("sw\t\t$t1,\t0($t2)")
            }
        } else if (thisOp == OpGetArray) {
            loadValue(thisCode->obj[2], $t0, false, isImm2, value2);
            bool isGo = false;
            auto *varSym = (VarSym *) symbolTable.getSymbolPtr(mips_search_map, thisCode->obj[1].str, isGo);
            string pp_reg = (isGo ? reg_gp : reg_fp);
            if (isImm2) {
                int arrayOffset = value2 + varSym->offset;
                int addr = isGo ? (arrayOffset * 4) : -(arrayOffset * 4);
                MIPS_CODE("lw\t\t$t1,\t" << to_string(addr) << "(" << pp_reg << ")")
            } else {
                int addr = isGo ? (varSym->offset * 4) : -(varSym->offset * 4);
                MIPS_CODE("addi\t$t2,\t" << pp_reg << ",\t" << to_string(addr))
                MIPS_CODE("sll\t\t$t0,\t$t0,\t2")
                if (isGo) {
                    MIPS_CODE("add\t\t$t2,\t$t2,\t$t0")
                } else {
                    MIPS_CODE("sub\t\t$t2,\t$t2,\t$t0")
                }
                MIPS_CODE("lw\t\t$t1,\t0($t2)")
            }
            saveValue(thisCode->obj[0].str, $t1);
        } else if (thisOp == OpPLUS) {
            loadValue(thisCode->obj[1], $t0, false, isImm1, value1);
            loadValue(thisCode->obj[2], $t1, false, isImm2, value2);
            if (isImm1 && isImm2) {
                MIPS_CODE("li\t\t$t2,\t" << to_string(value1 + value2))
            } else if (!isImm1 && isImm2) {
                MIPS_CODE("addi\t$t2,\t$t0,\t" << to_string(value2))
            } else if (isImm1 && !isImm2) {
                MIPS_CODE("addi\t$t2,\t$t1,\t" << to_string(value1))
            } else {
                MIPS_CODE("add\t\t$t2,\t$t0,\t$t1")
            }
            saveValue(thisCode->obj[0].str, $t2);
        } else if (thisOp == OpMINU) {
            loadValue(thisCode->obj[1], $t0, false, isImm1, value1);
            loadValue(thisCode->obj[2], $t1, false, isImm2, value2);
            if (isImm1 && isImm2) {
                MIPS_CODE("li\t\t$t2,\t" << to_string(value1 - value2))
            } else if (!isImm1 && isImm2) {
                MIPS_CODE("addi\t$t2,\t$t0,\t" << to_string(-value2))
            } else if (isImm1 && !isImm2) {
                MIPS_CODE("sub\t\t$t1,\t$0,\t$t1")
                MIPS_CODE("addi\t$t2,\t$t1,\t" << to_string(value1))
            } else {
                MIPS_CODE("sub\t\t$t2,\t$t0,\t$t1")
            }
            saveValue(thisCode->obj[0].str, $t2);
        } else if (thisOp == OpMULT) {
            loadValue(thisCode->obj[1], $t0, true, isImm1, value1);
            loadValue(thisCode->obj[2], $t1, true, isImm2, value2);
            MIPS_CODE("mult\t$t0,\t$t1")
            MIPS_CODE("mflo\t$t2")
            saveValue(thisCode->obj[0].str, $t2);
        } else if (thisOp == OpDIV) {
            loadValue(thisCode->obj[1], $t0, true, isImm1, value1);
            loadValue(thisCode->obj[2], $t1, true, isImm2, value2);
            MIPS_CODE("div\t\t$t0,\t$t1")
            MIPS_CODE("mflo\t$t2")
            saveValue(thisCode->obj[0].str, $t2);
        } else if (thisOp == OpExit) {
            MIPS_CODE("li\t\t$v0,\t10")
            MIPS_CODE("syscall")
        } else if (thisOp == OpJMain) {
            MIPS_CODE("j\tf_main")
        } else if (thisOp == OpLabel) {
            MIPS_PRINT(thisCode->obj[0].str << ":")
        } else if (thisOp == OpJmp) {
            MIPS_CODE("j\t\t" << thisCode->obj[0].str)
        } else if (thisOp == OpEQL || thisOp == OpNEQ) {
            loadValue(thisCode->obj[1], $t0, false, isImm1, value1);
            loadValue(thisCode->obj[2], $t1, false, isImm2, value2);
            string branch_model = (thisCode->p_next->op == OpBEZ ^ thisOp == OpNEQ) ? "bne\t\t" : "beq\t\t";
            if (isImm1 && isImm2) {
                if (value1 != value2) {
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
        }
    }
}

void MipsTranslator::saveValue(string &obj, const string &regName) {
    bool isGlobal = false;
    VarSym *varSym = (VarSym *) symbolTable.getSymbolPtr(mips_search_map, obj, isGlobal);
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
