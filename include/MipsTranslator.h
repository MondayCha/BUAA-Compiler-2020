//
// Created by dlely on 2020/11/13.
//

#ifndef MYCOMPILER_MIPSTRANSLATOR_H
#define MYCOMPILER_MIPSTRANSLATOR_H

#include "IRCodeManager.h"
#include "SymbolTable.h"

#define MIPS_PRINT(word) mipsFile << word << endl;
#define MIPS_CODE(code) mipsFile << "\t" << code << endl;

#define p_reg (isGlobal ? $gp : $fp)

#define RD1 thisCode->obj[0]
#define RS2 thisCode->obj[1]
#define RT3 thisCode->obj[2]

#define $t0 reg_t0
#define $t1 reg_t1
#define $t2 reg_t2
#define $a0 reg_a0
#define $gp reg_gp
#define $fp reg_fp
#define $v0 reg_v0

#define CONDITION(condition, str1, str2, str3, str4)\
if (branch_when_true) {\
if (isImm1 && isImm2) {\
if (condition) {\
MIPS_CODE("j\t" << (thisCode->p_next->obj[0].str))\
}\
} else if (!isImm1 && isImm2) {\
MIPS_CODE(str1 << "\t\t" << rs2s << ",\t" << to_string(value2) << ",\t" << (thisCode->p_next->obj[0].str))\
} else if (isImm1 && !isImm2) {\
MIPS_CODE(str2 << "\t\t" << rt3s << ",\t" << to_string(value1) << ",\t" << (thisCode->p_next->obj[0].str))\
} else {\
MIPS_CODE(str1 << "\t\t" << rs2s << ",\t" << rt3s << ",\t" << (thisCode->p_next->obj[0].str))\
}\
} else {\
if (isImm1 && isImm2) {\
if (!(condition)) {\
MIPS_CODE("j\t" << (thisCode->p_next->obj[0].str))\
}\
} else if (!isImm1 && isImm2) {\
MIPS_CODE(str3 << "\t\t" << rs2s << ",\t" << to_string(value2) << ",\t" << (thisCode->p_next->obj[0].str))\
} else if (isImm1 && !isImm2) {\
MIPS_CODE(str4 << "\t\t" << rt3s << ",\t" << to_string(value1) << ",\t" << (thisCode->p_next->obj[0].str))\
} else {\
MIPS_CODE(str3 << "\t\t" << rs2s << ",\t" << rt3s << ",\t" << (thisCode->p_next->obj[0].str))\
}\
}

class MipsTranslator {
private:
    string reg_t0 = "$t0";
    string reg_t1 = "$t1";
    string reg_t2 = "$t2";
    string reg_a0 = "$a0";
    string reg_gp = "$gp";
    string reg_fp = "$fp";
    string reg_v0 = "$v0";

    explicit MipsTranslator(ofstream &mipsFile, IRCodeManager &irCode, SymbolTable &symbolTable);

    ofstream &mipsFile;
    IRCodeManager &irCode;
    SymbolTable &symbolTable;
public:
    static MipsTranslator &getInstance(ofstream &mipsFile, IRCodeManager &irCode, SymbolTable &symbolTable);

    void translate();

    void loadValue(GoalObject &goalObject, string &regName, bool ifGen, bool &isImm, int &value);

    void saveValue(string &obj, const string &regName);

    int getTRegName(string &name);

    int getTRegFree(string &name);

    int getSRegName(string &name);

    int getSRegFree(string &name);
};


#endif //MYCOMPILER_MIPSTRANSLATOR_H
