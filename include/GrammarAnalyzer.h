//
// Created by Dlee on 2020/10/5.
//

#ifndef SCANNER_GRAMMARANALYZER_H
#define SCANNER_GRAMMARANALYZER_H

#include "Lexer.h"
#include "SymbolTable.h"

#define GET_TOKEN lexer.getNextToken()
#define PRINT_LEX {outFile << lexer.analyzerResult;}
#define PRINT_GET {outFile << lexer.analyzerResult;lexer.getNextToken();}
#define TYPE_IDEN (TOKEN_TYPE == INTTK || TOKEN_TYPE == CHARTK)
#define TYPE_ADDE (TOKEN_TYPE == PLUS || TOKEN_TYPE == MINU)
#define CHECK_GET(A, B) {\
if(TOKEN_TYPE != A){\
cerr << B;\
}\
PRINT_GET;\
}

#define TOKEN_TYPE lexer.typeCode


static string outputContent[30000];
static int p_content;

class GrammarAnalyzer {
private:
    explicit GrammarAnalyzer(Lexer &pronLexer, SymbolTable &pronSymbol, std::ofstream &pronFile);

    Lexer &lexer;
    SymbolTable &symbolTable;
    ofstream &outFile;
    ///////// analyze grammar //////////

public:
    static GrammarAnalyzer &getInstance(Lexer &pronLexer, SymbolTable &pronSymbol, std::ofstream &pronOutFile);

    void analyzeGrammar();

    void programAnalyzer();

    void constDeclarationAnalyzer();

    void constDefinitionAnalyzer();

    void intAssignAnalyzer();

    void integerAnalyzer();

    int unsignedIntAnalyzer();

    void charAssignAnalyzer();

    void characterAnalyzer();

    void varDeclarationAnalyzer();

    string varDefinitionAnalyzer(TypeCode typeCode);

    void reValFuncDefAnalyzer();

    void speReValFuncDefAnalyzer();

    void arrayVarAnalyzer(int &level, int &l1, int &l2);

    void arrayVarAnalyzer();

    void varInitAnalyzer(int level, int length1, int length2, bool isInteger);


    void constantAnalyzer(bool isInteger);

    void parameterTableAnalyzer();

    void paraTableEleAnalyzer();

    void compoundStatementAnalyzer();

    void varDeclareNoFuncAnalyzer();

    void statementColumnAnalyzer();

    bool statementAnalyzer();

    void loopStatementAnalyzer();

    void conditionAnalyzer();

    void expressionAnalyzer();

    void relationalOperatorAnalyzer();

    void itemAnalyzer();
};


#endif //SCANNER_GRAMMARANALYZER_H
