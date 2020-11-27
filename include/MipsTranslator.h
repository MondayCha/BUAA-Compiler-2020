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

#define $t0 reg_t0
#define $t1 reg_t1
#define $t2 reg_t2
#define $a0 reg_a0
#define $gp reg_gp
#define $fp reg_fp

class MipsTranslator {
private:
    const string reg_t0 = "$t0";
    const string reg_t1 = "$t1";
    const string reg_t2 = "$t2";
    const string reg_a0 = "$a0";
    const string reg_gp = "$gp";
    const string reg_fp = "$fp";

    explicit MipsTranslator(ofstream &mipsFile, IRCodeManager &irCode, SymbolTable &symbolTable);

    ofstream &mipsFile;
    IRCodeManager &irCode;
    SymbolTable &symbolTable;
public:
    static MipsTranslator &getInstance(ofstream &mipsFile, IRCodeManager &irCode, SymbolTable &symbolTable);
    void translate();

    void loadValue(GoalObject &goalObject, const string& regName, bool ifGen, bool &isImm, int &value);

    void saveValue(string &obj, const string& regName);
};


#endif //MYCOMPILER_MIPSTRANSLATOR_H
