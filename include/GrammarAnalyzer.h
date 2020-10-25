//
// Created by Dlee on 2020/10/5.
//

#ifndef SCANNER_GRAMMARANALYZER_H
#define SCANNER_GRAMMARANALYZER_H

#include "Lexer.h"
#include "SymbolTable.h"
#include "ErrorHandle.h"

// #define CLOSE_GRAMMER_OUTPUT

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
PRINT_G_ERR('l')\
}else{\
PRINT_GET\
}
#define CHECK_RBRACK \
if (TOKEN_TYPE != RBRACK) {\
PRINT_G_ERR('m')\
}else{\
PRINT_GET\
}
#define CHECK_SEMICN \
if (TOKEN_TYPE == SEMICN) {\
PRINT_GET\
} else {\
PRINT_G_SEM\
}
#define LEX_LINE lexer.lineNum
#define LEX_NAME lexer.strToken
#define LEX_LONA lexer.lexToken.content_p

#define TOKEN_TYPE lexer.typeCode
#define PRINT_G_ERR(A) errorHandle.printErrorLine(A, lexer.lineNum);
#define PRINT_G_SEM errorHandle.printErrorLine('k', lexer.lastLineNum);

class GrammarAnalyzer {
private:
    explicit GrammarAnalyzer(Lexer &pronLexer, SymbolTable &pronSymbol, ErrorHandle &pronError,
                             std::ofstream &pronFile);

    Lexer &lexer;
    SymbolTable &symbolTable;
    ErrorHandle &errorHandle;
    ofstream &outFile;
    ///////// analyze grammar //////////

public:
    static GrammarAnalyzer &
    getInstance(Lexer &pronLexer, SymbolTable &pronSymbol, ErrorHandle &pronError, std::ofstream &pronOutFile);

    void analyzeGrammar();

    void programAnalyzer();

    void constDeclarationAnalyzer();

    void constDefinitionAnalyzer();

    void intAssignAnalyzer();

    int integerAnalyzer();

    int unsignedIntAnalyzer();

    void charAssignAnalyzer();

    int characterAnalyzer();

    void varDeclarationAnalyzer();

    void varDefinitionAnalyzer(string &name, string &lowerName, TypeCode typeCode);

    void speReValFuncDefAnalyzer(string &name, string &lowerName, TypeCode tk);

    void arrayVarAnalyzer(int &level, int &l1, int &l2);

    void arrayVarAnalyzer();

    void varInitAnalyzer(int level, int length1, int length2, bool isInteger);

    bool constInVarInitAnalyzer_return_error(bool isInteger);

    void parameterTableAnalyzer(FuncSym &funcSym, bool declaration);

    void paraTableEleAnalyzer(FuncSym &funcSym, bool declaration);

    void compoundStatementAnalyzer();

    void varDeclareNoFuncAnalyzer();

    void statementColumnAnalyzer();

    bool statementAnalyzer();

    void loopStatementAnalyzer();

    void conditionAnalyzer();

    SymbolType expressionAnalyzer();

    SymbolType itemAnalyzer();

    void conditionalStatementAnalyzer();

    void arrayExpAssignAnalyzer();

    void assignStatementAnalyzer(string &lower_name);

    SymbolType factorAnalyzer();

    void returnStatementAnalyzer();

    void readStatementAnalyzer();

    void writeStatementAnalyzer();

    void functionDeclarationAnalyzer();

    SymbolType functionCallAnalyzer(string &lower_name);

    void valParaTableAnalyzer(vector<VarSym> &vectorVar);

    void stringAnalyzer();

    void switchStatementAnalyzer();

    void situationSwitchAnalyzer(SymbolType switchType);

    SymbolType constantAnalyzer();

    bool defaultSwitchAnalyzer();

    void readToRightBrack();
};


#endif //SCANNER_GRAMMARANALYZER_H
