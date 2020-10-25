//
// Created by Dlee on 2020/9/22.
//

#ifndef SCANNER_LEXER_H
#define SCANNER_LEXER_H

#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include <cctype>
#include "typeList.h"
#include "ErrorHandle.h"

#define LEX_TOKEN_LEN 233
// if use (isalpha(_C) | (_C == '_'))
// use of a signed integer operand with a binary bitwise operator
#define isalund(_C) (isalpha(_C) || (_C == '_'))

#define PRINT_ERR(A) errorHandle.printErrorLine(A, lineNum, findError);

using namespace std;

class Lexer {
private:
    explicit Lexer(ErrorHandle &errorHandle, ifstream &inputFile, ofstream &outputFile);   // Singleton Pattern
    ErrorHandle &errorHandle;
    ifstream &inFile;               // The copy function is protected, so track back
    ofstream &outFile;
    int lineNum;
    int lastLineNum;
    string strToken;
    TypeCode typeCode;
    struct LexToken {
        string content_p;
        int contentNum;
    } lexToken{};
    string analyzerResult;

    friend class GrammarAnalyzer;

public:
    static Lexer &getInstance(ErrorHandle &errorHandle, ifstream &inputFile, ofstream &outputFile);

    static string getTypeStr(TypeCode typeCode);

    void getNextToken();

    bool meetEOF();
};

#endif //SCANNER_LEXER_H
