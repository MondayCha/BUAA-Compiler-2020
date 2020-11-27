//
// Created by Dlee on 2020/10/5.
//

#ifndef SCANNER_GRAMMARANALYZER_H
#define SCANNER_GRAMMARANALYZER_H

#include "Lexer.h"
#include "typeList.h"
#include "SymbolTable.h"
#include "ErrorHandle.h"
#include "IRCodeManager.h"


//#define CLOSE_GRAMMER_OUTPUT

#define GET_TOKEN lexer.getNextToken()

#ifdef CLOSE_GRAMMER_OUTPUT
#define PRINT_GET lexer.getNextToken();
#define PRINT_MES(Message)
#else
#define PRINT_GET outFile << lexer.analyzerResult << endl;lexer.getNextToken();
#define PRINT_MES(Message) outFile << Message << endl;
#endif
#define TYPE_IDEN (TOKEN_TYPE == INTTK || TOKEN_TYPE == CHARTK)
#define TYPE_ADDE (TOKEN_TYPE == PLUS || TOKEN_TYPE == MINU)
#define CHECK_GET(A, B) {\
if(TOKEN_TYPE != A){\
cerr << B << " and line num is " << lexer.lineNum << endl;}\
PRINT_GET;\
}
#define CHECK_RPARENT \
if (TOKEN_TYPE != RPARENT) {\
errorHandle.printErrorLine('l', lexer.lastLineNum);\
}else{\
PRINT_GET\
}

#define CHECK_RBRACK \
if (TOKEN_TYPE != RBRACK) {\
errorHandle.printErrorLine('m', lexer.lastLineNum);\
}else{\
PRINT_GET\
}
#define CHECK_SEMICN \
if (TOKEN_TYPE == SEMICN) {\
PRINT_GET\
} else {\
PRINT_G_SEM\
}

#define STORE_EXP(opType, obj1, obj2) \
ans = genTmpVar_and_insert();           \
irCode.addThreeAddCode(new ThreeAddCode(opType, ans, obj1, obj2));\
op1 = move(ans);

#define LEX_LINE lexer.lineNum
#define LEX_NAME lexer.strToken
#define LEX_LONA lexer.lexToken.content_p

#define TOKEN_TYPE lexer.typeCode
#define PRINT_G_ERR(A) errorHandle.printErrorLine(A, lexer.lineNum);
#define PRINT_G_SEM errorHandle.printErrorLine('k', lexer.lastLineNum);
#define ADD_MIDCODE(op,s1,s2,s3) irCode.addThreeAddCode(new ThreeAddCode(op, s1, s2, s3));

class GrammarAnalyzer {
private:
    explicit GrammarAnalyzer(Lexer &pronLexer, SymbolTable &pronSymbol, ErrorHandle &pronError, IRCodeManager &pronCode,
                             std::ofstream &pronFile);

    Lexer &lexer;
    SymbolTable &symbolTable;
    ErrorHandle &errorHandle;
    IRCodeManager &irCode;
    ofstream &outFile;
    ///////// analyze grammar //////////

public:
    static GrammarAnalyzer &
    getInstance(Lexer &pronLexer, SymbolTable &pronSymbol, ErrorHandle &pronError, IRCodeManager &pronCode,
                std::ofstream &pronOutFile);

    void analyzeGrammar();

    void programAnalyzer();

    void constDeclarationAnalyzer();

    void constDefinitionAnalyzer();

    void intAssignAnalyzer();

    int integerAnalyzer();

    int integerAnalyzer(bool &checkBit);

    int unsignedIntAnalyzer();

    void charAssignAnalyzer();

    int characterAnalyzer();

    void varDeclarationAnalyzer();

    void varDefinitionAnalyzer(string &name, string &lowerName, TypeCode typeCode);

    void speReValFuncDefAnalyzer(string &name, string &lowerName, TypeCode tk);

    void arrayVarAnalyzer(int &level, int &l1, int &l2);

    void arrayVarAnalyzer();

    void varInitAnalyzer(string &lowerName, int level, int length1, int length2, int offset, bool isInteger);

    int constInVarInitAnalyzer_return_value(bool isInteger);

    void parameterTableAnalyzer(FuncSym &funcSym, bool declaration);

    void paraTableEleAnalyzer(FuncSym &funcSym, bool declaration);

    void compoundStatementAnalyzer();

    void varDeclareNoFuncAnalyzer();

    void statementColumnAnalyzer();

    void statementAnalyzer();

    void loopStatementAnalyzer();

    void conditionAnalyzer(string &condition_var);

    SymbolType expressionAnalyzer(string &exp_str, int &exp_int);

    SymbolType itemAnalyzer(string &exp_str, int &exp_int);

    SymbolType factorAnalyzer(string &exp_str, int &exp_int);

    string genTmpVar_and_insert();

    void conditionalStatementAnalyzer();

    string arrayExpAssignAnalyzer(string &lower_name, bool genTmp, string &index_s, int &index);

    void assignStatementAnalyzer(string &name, string &lower_name);

    void returnStatementAnalyzer();

    void readStatementAnalyzer();

    void writeStatementAnalyzer();

    void functionDeclarationAnalyzer();

    SymbolType functionCallAnalyzer(string &lower_name);

    void valParaTableAnalyzer(vector<VarSym> &vectorVar);

    string stringAnalyzer();

    void switchStatementAnalyzer();

    void situationSwitchAnalyzer(SymbolType switchType);

    SymbolType constantAnalyzer();

    bool defaultSwitchAnalyzer();

    void readToRightBrack_and_log_error();

    void endConstOrVarDeclaration();

    static int getNeedSpace(int level, int length1, int length2);

    SymbolType expressionAnalyzer(string &exp_str);

    SymbolType expressionAnalyzer();

    string arrayExpAssignAnalyzer(string &lower_name);

    string genLabel();
};

#endif //SCANNER_GRAMMARANALYZER_H
