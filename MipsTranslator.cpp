//
// Created by dlely on 2020/11/13.
//

#include "MipsTranslator.h"
#include "GrammarAnalyzer.h"
#include <fstream>
#include <stack>

map<string, Symbol *> *mips_search_map;
int tRegBusy[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
int sRegBusy[9] = {0, 0, 0, 0, 0, 0, 0, 0};
string tRegName[10];
string sRegName[9];
int divLabelOffset;

MipsTranslator::MipsTranslator(ofstream &mipsFile, IRCodeManager &irCode, ActiveOptimize &pronActive,
                               SymbolTable &symbolTable)
        : mipsFile(mipsFile), irCode(irCode), activeOpt(pronActive), symbolTable(symbolTable) {
    divLabelOffset = 0;
}

MipsTranslator &MipsTranslator::getInstance(ofstream &mipsFile, IRCodeManager &irCode, ActiveOptimize &pronActive,
                                            SymbolTable &symbolTable) {
    static MipsTranslator initializer(mipsFile, irCode, pronActive, symbolTable);
    return initializer;
}

#ifdef CODE_OPTIMIZE_ON

void MipsTranslator::translateCode(FuncSym *funcPtr, const list<ThreeAddCode *> &codeList) {
    int value1, value2;
    bool isImm1, isImm2, isGlobal, saveToMemory, isDirty;
    FuncSym *mips_p_current_func = funcPtr;
    mips_search_map = &(funcPtr->funcLocalTable);
    stack<ThreeAddCode *> funcParamsStack;
    ////////////////// sReg ////////////////////
    int sRegNum = 0;
    while (!funcPtr->varUseCountQueue.empty() && sRegNum < 8) {
        string varName = funcPtr->varUseCountQueue.top().name;
        funcPtr->varUseCountQueue.pop();
        isGlobal = false;
        Symbol *p_symbol = symbolTable.getSymbolPtr(&funcPtr->funcLocalTable, varName, isGlobal);
        if (!isGlobal) {
            sRegBusy[sRegNum] = 1;
            sRegName[sRegNum] = varName;
            cout << "\t\t\t\t# " << "allocate " << sRegName[sRegNum] << " to $s" << to_string(sRegNum) << endl;
            sRegNum++;
        }
    }
    while (sRegNum < 8) {
        sRegBusy[sRegNum] = 0;
        sRegNum++;
    }
    ////////////////////////////////////////////
    for (auto thisCode : codeList) {
        irCode.ircodeFile << thisCode->toString() << endl;
        if (true) {
            MIPS_PRINT("############ " << thisCode->toString())
        }
        OperatorType thisOp = thisCode->op;
        isImm1 = false, isImm2 = false, isGlobal = false, isDirty = false, saveToMemory = true;
        string rs2s = "$t0", rt3s = "$t1", rd1s = "$t2";
        if (thisOp == OpBEZ || thisOp == OpBNEZ) {
            continue;
        } else if (thisOp == OpPLUS) {
            loadValue(RS2, rs2s, false, isImm1, value1);
            loadValue(RT3, rt3s, false, isImm2, value2);
            PMMD_REG_JUDGE
            // 常数传递
            if (isImm1 && isImm2) {
                MIPS_CODE("li\t\t" << rd1s << ",\t" << to_string(value1 + value2))
            } else if (!isImm1 && isImm2) {
                MIPS_CODE("addiu\t" << rd1s << ",\t" << rs2s << ",\t" << to_string(value2))
            } else if (isImm1 && !isImm2) {
                MIPS_CODE("addiu\t" << rd1s << ",\t" << rt3s << ",\t" << to_string(value1))
            } else {
                MIPS_CODE("addu\t\t" << rd1s << ",\t" << rs2s << ",\t" << rt3s)
            }
            // 存入内存
            if (saveToMemory) { saveValue(RD1.str, rd1s); }
        } else if (thisOp == OpMINU) {
            loadValue(RS2, rs2s, false, isImm1, value1);
            loadValue(RT3, rt3s, false, isImm2, value2);
            PMMD_REG_JUDGE
            // 常数传递
            if (isImm1 && isImm2) {
                MIPS_CODE("li\t\t" << rd1s << ",\t" << to_string(value1 - value2))
            } else if (!isImm1 && isImm2) {
                MIPS_CODE("addiu\t" << rd1s << ",\t" << rs2s << ",\t" << to_string(-value2))
            } else if (isImm1 && !isImm2) {
                if (value1 == 0) {
                    MIPS_CODE("sub\t\t" << rd1s << ",\t$0,\t" << rt3s)
                } else {
                    MIPS_CODE("addiu\t" << rd1s << ",\t" << rt3s << ",\t" << to_string(-value1))
                    MIPS_CODE("sub\t\t" << rd1s << ",\t$0,\t" << rd1s)
                }
            } else {
                MIPS_CODE("sub\t\t" << rd1s << ",\t" << rs2s << ",\t" << rt3s)
            }
            if (saveToMemory) { saveValue(RD1.str, rd1s); }
        } else if (thisOp == OpMULT) {
            loadValue(RS2, rs2s, false, isImm1, value1);
            loadValue(RT3, rt3s, false, isImm2, value2);
            PMMD_REG_JUDGE
            // 常数传递
            if (isImm1 && isImm2) {
                MIPS_CODE("li\t\t" << rd1s << ",\t" << to_string(value1 * value2))
            } else if (!isImm1 && !isImm2) {
                MIPS_CODE("mul\t" << rd1s << ",\t" << rs2s << ",\t" << rt3s)
            } else {
                string *tmpMulReg = isImm1 ? &rt3s : &rs2s;
                int tmpMulValue = isImm2 ? value2 : value1;
                bool tmpMulValueLss = tmpMulValue < 0;
                unsigned int tmpMulValueAbs = tmpMulValueLss ? -tmpMulValue : tmpMulValue;
                bool isTwoPower = (tmpMulValueAbs & (tmpMulValueAbs - 1)) == 0;
                if (tmpMulValue == 0) {
                    MIPS_CODE("li\t\t" << rd1s << ",\t0")
                } else if (isTwoPower) {
                    // 获得一个整数的二进制位数
                    unsigned int binaryOfValue = 1;
                    while ((tmpMulValueAbs >> binaryOfValue) > 0) {
                        binaryOfValue++;
                    }
                    MIPS_CODE("sll\t\t" << rd1s << ",\t" << *tmpMulReg << ",\t" << to_string(binaryOfValue - 1))
                    if (tmpMulValueLss) {
                        MIPS_CODE("sub\t\t" << rd1s << ",\t$0,\t" << rd1s)
                    }
                } else {
                    string *tmpAnoMulReg = isImm2 ? &rt3s : &rs2s;
                    MIPS_CODE("li\t" << *tmpAnoMulReg << ",\t" << to_string(tmpMulValue))
                    MIPS_CODE("mul\t" << rd1s << ",\t" << rs2s << ",\t" << rt3s)
                }
            }
            if (saveToMemory) { saveValue(RD1.str, rd1s); }
        } else if (thisOp == OpDIV) {
            loadValue(RS2, rs2s, false, isImm1, value1);
            loadValue(RT3, rt3s, false, isImm2, value2);
            PMMD_REG_JUDGE
            // 常数传递
//            MIPS_CODE("div\t\t" << rs2s << ",\t" << rt3s)
//            MIPS_CODE("mflo\t" << rd1s)
            if (isImm1 && isImm2) {
                MIPS_CODE("li\t\t" << rd1s << ",\t" << to_string(value1 / value2))
            } else if (!isImm1 && !isImm2) {
                MIPS_CODE("div\t\t" << rs2s << ",\t" << rt3s)
                MIPS_CODE("mflo\t" << rd1s)
            } else if (isImm1 && !isImm2) {
                if (value1 == 0) {
                    MIPS_CODE("move\t" << rd1s << ",\t$0")
                } else {
                    MIPS_CODE("li\t\t" << rs2s << ",\t" << to_string(value1))
                    MIPS_CODE("div\t\t" << rs2s << ",\t" << rt3s)
                    MIPS_CODE("mflo\t" << rd1s)
                }
            } else {
                bool isImm2Gre = value2 > 0;
                unsigned int tmpDivValueAbs = isImm2Gre ? value2 : -value2;
                bool isTwoPower = (tmpDivValueAbs & (tmpDivValueAbs - 1)) == 0;
                if (value2 == 1) {
                    MIPS_CODE("move\t" << rd1s << ",\t" << rs2s)
                } else if (value2 == 0 || !isTwoPower) {
                    MIPS_CODE("li\t\t" << rt3s << ",\t" << to_string(value2))
                    MIPS_CODE("div\t\t" << rs2s << ",\t" << rt3s)
                    MIPS_CODE("mflo\t" << rd1s)
                } else {
                    string labelDiv = "l_div_" + to_string(divLabelOffset++);
                    if (isImm2Gre) {
                        MIPS_CODE("move\t$t0,\t" << rs2s)
                    } else {
                        MIPS_CODE("sub\t\t$t0,\t$0,\t" << rs2s)
                    }
                    MIPS_CODE("bgtz\t$t0,\t" << labelDiv)
                    // 获得一个整数的二进制位数
                    unsigned int binaryOfValue = 1;
                    while ((tmpDivValueAbs >> binaryOfValue) > 0) {
                        binaryOfValue++;
                    }
                    MIPS_CODE("addiu\t$t0,\t$t0,\t" << to_string(tmpDivValueAbs - 1))
                    MIPS_PRINT(labelDiv << ":")
                    MIPS_CODE("sra\t\t" << rd1s << ",\t$t0,\t" << to_string(binaryOfValue - 1))
                }
            }
            if (saveToMemory) { saveValue(RD1.str, rd1s); }
        } else if (thisOp == OpLabel) {
            MIPS_PRINT(RD1.str << ":")
        } else if (thisOp == OpScanf) {
            auto *varSym = (VarSym *) symbolTable.getSymbolPtr(mips_search_map,
                                                               RD1.str, isGlobal);
            if (varSym->symbolType == INT) {
                MIPS_CODE("li\t\t$v0,\t5")
            } else {
                MIPS_CODE("li\t\t$v0,\t12")
            }
            MIPS_CODE("syscall")
            int srdRegNum = getSRegName(RD1.str);
            if (srdRegNum != -1) {
                MIPS_CODE("move\t$s" << to_string(srdRegNum) << ",\t$v0")
            } else {
                int address = isGlobal ? (varSym->offset * 4) : -(varSym->offset * 4);
                string pp_reg = p_reg;
                MIPS_CODE("sw\t\t$v0,\t" << to_string(address) << "(" << pp_reg << ")")
            }
        } else if (thisOp == OpPrint) {
            SymbolType printType = RD1.type;
            switch (printType) {
                case VOID:
                    // 字符串
                    MIPS_CODE("la\t\t$a0,\t" << (RS2).str)
                    MIPS_CODE("li\t\t$v0,\t4")
                    break;
                default:
                    string a0Value = "$a0";
                    loadValue(RS2, a0Value, true, isImm1, value1, isDirty);
                    if (isDirty) {
                        MIPS_CODE("move\t$a0,\t" << a0Value);
                    }
                    int syscallNum = (printType == INT) ? 1 : 11;
                    MIPS_CODE("li\t\t$v0,\t" << to_string(syscallNum));
            }
            MIPS_CODE("syscall")
        } else if (thisOp == OpASSIGN) {
            // 中间变量不可能是赋值语句的RD1
            if (RD1.str != RS2.str) {
                int srdRegNum = getSRegName(RD1.str);
                if (srdRegNum != -1) {
                    saveToMemory = false;
                    rd1s = "$s" + to_string(srdRegNum);
                    loadValue(RS2, rd1s, true, isImm1, value1, isDirty);
                    if (isDirty) {
                        MIPS_CODE("move\t$s" << to_string(srdRegNum) << ",\t" << rd1s);
                    }
                } else {
                    loadValue(RS2, rs2s, true, isImm1, value1);
                    saveValue(RD1.str, rs2s);
                }
            }
        } else if (thisOp == OpAssArray) {
            // AssArr  [string  : a]   [string  : tmp_12]      [int/char: 1]
            loadValue(RT3, rt3s, false, isImm2, value2);
            loadValue(RS2, rs2s, false, isImm1, value1);
            bool isGo = false;
            auto *varSym = (VarSym *) symbolTable.getSymbolPtr(mips_search_map, RD1.str, isGo);
            string pp_reg = (isGo ? reg_gp : reg_fp);
            if (isImm1) {
                // 数组下标存放在value1中
                int addr = isGo ? ((value1 + varSym->offset) * 4) : -((value1 + varSym->offset) * 4);
                if (isImm2) {
                    if (value2 == 0) {
                        MIPS_CODE("sw\t\t$0,\t" << to_string(addr) << "(" << pp_reg << ")")
                    } else {
                        MIPS_CODE("li\t\t" << rt3s << ",\t" << to_string(value2))
                        MIPS_CODE("sw\t\t" << rt3s << ",\t" << to_string(addr) << "(" << pp_reg << ")")
                    }
                } else {
                    MIPS_CODE("sw\t\t" << rt3s << ",\t" << to_string(addr) << "(" << pp_reg << ")")
                }
            } else {
                int addr = isGo ? (varSym->offset * 4) : -(varSym->offset * 4);
                MIPS_CODE("addiu\t$t2,\t" << pp_reg << ",\t" << to_string(addr))
                MIPS_CODE("sll\t\t$t0,\t" << rs2s << ",\t2")
                if (isGo) {
                    MIPS_CODE("addu\t\t$t2,\t$t2,\t$t0")
                } else {
                    MIPS_CODE("sub\t\t$t2,\t$t2,\t$t0")
                }
                if (isImm2) {
                    if (value2 == 0) {
                        MIPS_CODE("sw\t\t$0,\t0($t2)")
                    } else {
                        MIPS_CODE("li\t\t" << rt3s << ",\t" << to_string(value2))
                        MIPS_CODE("sw\t\t" << rt3s << ",\t0($t2)")
                    }
                } else {
                    MIPS_CODE("sw\t\t" << rt3s << ",\t0($t2)")
                }
            }
        } else if (thisOp == OpGetArray) {
            loadValue(RT3, rt3s, false, isImm2, value2);
            auto *varSym = (VarSym *) symbolTable.getSymbolPtr(mips_search_map, RS2.str, isGlobal);
            string pp_reg = (isGlobal ? reg_gp : reg_fp);
            PMMD_REG_JUDGE
            if (isImm2) {
                int arrayOffset = value2 + varSym->offset;
                int addr = isGlobal ? (arrayOffset * 4) : -(arrayOffset * 4);
                MIPS_CODE("lw\t\t" << rd1s << ",\t" << to_string(addr) << "(" << pp_reg << ")")
            } else {
                int addr = isGlobal ? (varSym->offset * 4) : -(varSym->offset * 4);
                MIPS_CODE("addiu\t$t2,\t" << pp_reg << ",\t" << to_string(addr))
                MIPS_CODE("sll\t\t$t0,\t" << rt3s << ",\t2")
                if (isGlobal) {
                    MIPS_CODE("addu\t$t2,\t$t2,\t$t0")
                } else {
                    MIPS_CODE("sub\t\t$t2,\t$t2,\t$t0")
                }
                MIPS_CODE("lw\t\t" << rd1s << ",\t0($t2)")
            }
            if (saveToMemory) { saveValue(RD1.str, rd1s); }
            // saveValue(RD1.str, rd1s);
        } else if (thisOp == OpEQL || thisOp == OpNEQ) {
            loadValue(RS2, rs2s, false, isImm1, value1);
            loadValue(RT3, rt3s, false, isImm2, value2);
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
                    MIPS_CODE("li\t\t" << rt3s << ",\t" << to_string(value2))
                } else if (isImm1 && !isImm2) {
                    MIPS_CODE("li\t\t" << rs2s << ",\t" << to_string(value1))
                }
                MIPS_CODE(branch_model << rs2s << ",\t" << rt3s << ",\t" << (thisCode->p_next->obj[0].str))
            }
        } else if (thisOp == OpLSS || thisOp == OpLEQ || thisOp == OpGRE || thisOp == OpGEQ) {
            loadValue(RS2, rs2s, false, isImm1, value1);
            loadValue(RT3, rt3s, false, isImm2, value2);
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
            if (RS2.type != VOID) {
                string v0Value = "$v0";
                loadValue(RD1, v0Value, true, isImm1, value1, isDirty);
                if (isDirty) {
                    MIPS_CODE("move\t$v0,\t" << v0Value);
                }
            }
            MIPS_CODE("jr\t\t$ra")
        } else if (thisOp == OpRetVar) {
            int srdRegNum;
            if (RD1.str[0] == '@') {
                // RD1 是临时变量
                int rdRegNum = getTRegFree(RD1.str);
                if (rdRegNum != -1) {
                    rd1s = "$t" + to_string(rdRegNum);
                    MIPS_CODE("move\t" << rd1s << ",\t$v0")
                    continue;
                }
            } else if ((srdRegNum = getSRegName(RD1.str)) > -1) {
                MIPS_CODE("move\t$s" << to_string(srdRegNum) << ",\t$v0")
                continue;
            }
            int retOffset = ((VarSym *) symbolTable.getSymbolPtr(mips_search_map, RD1.str, isGlobal))->offset;
            if (isGlobal) {
                MIPS_CODE("sw\t\t$v0,\t" << to_string(4 * retOffset) << "($gp)")
            } else {
                MIPS_CODE("sw\t\t$v0,\t" << to_string(-4 * retOffset) << "($fp)")
            }
        } else if (thisOp == OpPaVal) {
            funcParamsStack.push(thisCode);
        } else if (thisOp == OpCall) {
            ThreeAddCode *paramsPtr;
            FuncSym *callFuncPtr = symbolTable.getFuncPtr(RD1.str);
            int callFuncParamsSize = callFuncPtr->parameters.size();
            while (callFuncParamsSize > 0) {
                rs2s = "$t0";
                paramsPtr = funcParamsStack.top();
                funcParamsStack.pop();
                loadValue(paramsPtr->obj[1], rs2s, true, isImm1, value1);
                callFuncParamsSize--;
                MIPS_CODE("sw\t\t" << rs2s << ",\t" << to_string(-4 * callFuncParamsSize) << "($sp)")
            }
            // 保存临时变量
            vector<string> funcVarList;
            for (int i = 3; i < 10; i++) {
                if (tRegBusy[i]) {
                    funcVarList.push_back("$t" + to_string(i));
                }
            }
            for (int i = 0; i < 8; ++i) {
                if (sRegBusy[i]) {
                    funcVarList.push_back("$s" + to_string(i));
                }
            }
            int funcOffset = 4 * (callFuncPtr->funcSpace + funcVarList.size()) + 8;
            MIPS_CODE("addiu\t$sp,\t$sp,\t" << to_string(-funcOffset))
            MIPS_CODE("sw\t\t$ra,\t4($sp)")
            MIPS_CODE("sw\t\t$fp,\t8($sp)")
            for (int i = 0; i < funcVarList.size(); ++i) {
                MIPS_CODE("sw\t\t" << funcVarList[i] << ",\t" << to_string(12 + 4 * i) << "($sp)")
            }
            MIPS_CODE("addiu\t$fp,\t$sp,\t" << to_string(funcOffset))
            MIPS_CODE("jal\t\tf_" << RD1.str)
            for (int i = 0; i < funcVarList.size(); ++i) {
                MIPS_CODE("lw\t\t" << funcVarList[i] << ",\t" << to_string(12 + 4 * i) << "($sp)")
            }
            MIPS_CODE("lw\t\t$fp,\t8($sp)")
            MIPS_CODE("lw\t\t$ra,\t4($sp)")
            MIPS_CODE("addiu\t$sp,\t$sp,\t" << to_string(funcOffset))
        } else if (thisOp == OpParam) {
            int sRegNum = getSRegName(RS2.str);
            if (sRegNum > -1) {
                Symbol *p_symbol = symbolTable.getSymbolPtr(mips_search_map, RS2.str, isGlobal);
                int address = -(((VarSym *) p_symbol)->offset * 4);
                MIPS_CODE("lw\t\t$s" << to_string(sRegNum) << ",\t" << to_string(address) << "($fp)")
            }
        } else if (thisOp == OpFunc) {
            mips_p_current_func = symbolTable.getFuncPtr(RS2.str);
            mips_search_map = &(mips_p_current_func->funcLocalTable);
            MIPS_PRINT("f_" << mips_p_current_func->lowerName << ":")
            if (mips_p_current_func->symbolType == MAIN) {
                MIPS_CODE("move\t$fp,\t$sp")
                MIPS_CODE("addiu\t$sp,\t$sp,\t" << to_string(-4 * (mips_p_current_func->funcSpace) - 8))
            }
            mips_search_map = &(mips_p_current_func->funcLocalTable);
        } else if (thisOp == OpExit) {
            MIPS_CODE("li\t\t$v0,\t10")
            MIPS_CODE("syscall")
        } else if (thisOp == OpJMain) {
            MIPS_CODE("j\tf_main")
        } else if (thisOp == OpJmp) {
            MIPS_CODE("j\t\t" << RD1.str)
        }
    }
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
    // global var and const
    translateCode(activeOpt.globalPtr, activeOpt.globalPtr->funcIRCodeList);
    auto itera = activeOpt.usedFuncMap.begin();
    while (itera != activeOpt.usedFuncMap.end()) {
        translateCode(itera->second, (itera->second)->funcIRCodeList);
        MIPS_CODE("jr\t$ra")
        itera++;
    }
    translateCode(activeOpt.mainPtr, activeOpt.mainPtr->funcIRCodeList);
}

#else
void MipsTranslator::translate() {
    MIPS_PRINT(".data")
    for (auto s : irCode.getStringDataList()) {
        MIPS_CODE(s.label << ":\t" << ".asciiz\t\"" << s.content << "\"")
    }
    MIPS_CODE("nLine:\t.asciiz\t\"\\n\"")
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
        } else if (thisOp == OpFunc) {
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
            loadValue(RT3, $t1, true, isImm2, value2);
            loadValue(RS2, $t0, false, isImm1, value1);
            bool isGo = false;
            auto *varSym = (VarSym *) symbolTable.getSymbolPtr(mips_search_map, RD1.str, isGo);
            string pp_reg = (isGo ? reg_gp : reg_fp);
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
            if (RS2.type != VOID) {
                loadValue(RD1, $v0, true, isImm1, value1);
            }
            MIPS_CODE("jr\t\t$ra")
        } else if (thisOp == OpRetVar) {
            int retOffset = ((VarSym *) symbolTable.getSymbolPtr(mips_search_map, RD1.str, isGlobal))->offset;
            if (isGlobal) {
                MIPS_CODE("sw\t\t$v0,\t" << to_string(4 * retOffset) << "($gp)")
            } else {
                MIPS_CODE("sw\t\t$v0,\t" << to_string(-4 * retOffset) << "($fp)")
            }
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
#endif

void MipsTranslator::saveValue(string &obj, const string &regName) {
    bool isGlobal = false;
    auto *varSym = (VarSym *) symbolTable.getSymbolPtr(mips_search_map, obj, isGlobal);
    int address = isGlobal ? (varSym->offset * 4) : -(varSym->offset * 4);
    string pp_reg = p_reg;
    MIPS_CODE("sw\t\t" << regName << ",\t" << to_string(address) << "(" << pp_reg << ")")
}

int MipsTranslator::getTRegName(string &name) {
    for (int i = 3; i < 10; ++i) {
        if (tRegBusy[i] && tRegName[i] == name) {
            mipsFile << "\t\t\t\t# " << "use " << name << " in $t" << to_string(i) << endl;
            return i;
        }
    }
    return -1;
}

int MipsTranslator::getTRegFree(string &name) {
    for (int i = 3; i < 10; ++i) {
        if (!tRegBusy[i]) {
            tRegBusy[i] = 1;
            tRegName[i] = name;
            mipsFile << "\t\t\t\t# " << "allocate " << name << " to $t" << to_string(i) << endl;
            return i;
        }
    }
    return -1;
}

int MipsTranslator::getSRegName(string &name) {
    for (int i = 0; i < 8; ++i) {
        if (sRegBusy[i] && sRegName[i] == name) {
            mipsFile << "\t\t\t\t# " << "use " << name << " in $s" << to_string(i) << endl;
            return i;
        }
    }
    return -1;
}

int MipsTranslator::getSRegFree(string &name) {
    for (int i = 0; i < 8; ++i) {
        if (!sRegBusy[i]) {
            sRegBusy[i] = 1;
            sRegName[i] = name;
            mipsFile << "\t\t\t\t# " << "allocate " << name << " to $s" << to_string(i) << endl;
            return i;
        }
    }
    return -1;
}

void MipsTranslator::loadValue(GoalObject &goalObject, string &regName, bool ifGen, bool &isImm, int &value) {
    bool isGlobal = false;
    if (goalObject.branch == 2) {
        // 立即数
        if (ifGen) {
            MIPS_CODE("li\t\t" << regName << ",\t" << goalObject.str)
        }
        value = goalObject.num;
        isImm = true;
    } else {
        Symbol *p_symbol = symbolTable.getSymbolPtr(mips_search_map, goalObject.str, isGlobal);
        if (p_symbol != nullptr) {
            if (p_symbol->symbolAtt == CON) {
                if (ifGen) {
                    MIPS_CODE("li\t\t" << regName << ",\t" << to_string(((ConSym *) p_symbol)->content));
                }
                value = ((ConSym *) p_symbol)->content;
                isImm = true;
            } else {                                // 全局变量/局部变量/中间变量
                if (goalObject.str[0] == '@') {     // 中间变量
                    int tRegNum = getTRegName(goalObject.str);
                    if (tRegNum != -1) {
                        regName = "$t" + to_string(tRegNum);
                        tRegBusy[tRegNum] = 0;
                        return;
                    }
                } else if (!isGlobal) {                // 局部变量
                    int sRegNum = getSRegName(goalObject.str);
                    if (sRegNum != -1) {
                        regName = "$s" + to_string(sRegNum);
                        return;
                    }
                }
                // 全局变量/局部变量
                int address = isGlobal ? (((VarSym *) p_symbol)->offset * 4) : -(((VarSym *) p_symbol)->offset * 4);
                string pp_reg = p_reg;
                MIPS_CODE("lw\t\t" << regName << ",\t" << to_string(address) << "(" << pp_reg << ")")
            }
        } else {
            // 立即数但保存出错了
            if (goalObject.str.length() > 0) {
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
}

void MipsTranslator::loadValue(GoalObject &goalObject, string &regName, bool ifGen, bool &isImm, int &value,
                               bool &regChange) {
    bool isGlobal = false;
    if (goalObject.branch == 2) {
        // 立即数
        if (ifGen) {
            MIPS_CODE("li\t\t" << regName << ",\t" << goalObject.str)
        }
        value = goalObject.num;
        isImm = true;
    } else {
        Symbol *p_symbol = symbolTable.getSymbolPtr(mips_search_map, goalObject.str, isGlobal);
        if (p_symbol != nullptr) {
            if (p_symbol->symbolAtt == CON) {
                if (ifGen) {
                    MIPS_CODE("li\t\t" << regName << ",\t" << to_string(((ConSym *) p_symbol)->content));
                }
                value = ((ConSym *) p_symbol)->content;
                isImm = true;
            } else {                                // 全局变量/局部变量/中间变量
                if (goalObject.str[0] == '@') {     // 中间变量
                    int tRegNum = getTRegName(goalObject.str);
                    if (tRegNum != -1) {
                        regName = "$t" + to_string(tRegNum);
                        regChange = true;
                        tRegBusy[tRegNum] = 0;
                        return;
                    }
                } else if (!isGlobal) {                // 局部变量
                    int sRegNum = getSRegName(goalObject.str);
                    if (sRegNum != -1) {
                        regName = "$s" + to_string(sRegNum);
                        regChange = true;
                        return;
                    }
                }
                // 全局变量/局部变量
                int address = isGlobal ? (((VarSym *) p_symbol)->offset * 4) : -(((VarSym *) p_symbol)->offset * 4);
                string pp_reg = p_reg;
                MIPS_CODE("lw\t\t" << regName << ",\t" << to_string(address) << "(" << pp_reg << ")")
            }
        } else {
            // 正立即数但保存出错了
            if (goalObject.str.length() > 0) {
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
}
