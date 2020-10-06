//
// Created by Dlee on 2020/9/22.
//

#ifndef SCANNER_LEXER_H
#define SCANNER_LEXER_H

#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include <ctype.h>
#include "typeList.h"

#define LEX_TOKEN_LEN 233
// if use (isalpha(_C) | (_C == '_'))
// use of a signed integer operand with a binary bitwise operator
#define isalund(_C) (isalpha(_C) || (_C == '_'))

using namespace std;

class Lexer {
private:
    explicit Lexer(ifstream &inputFile, ofstream &outputFile);   // Singleton Pattern
    ifstream &inFile;               // The copy function is protected, so track back
    ofstream &outFile;
    int lineNum;
    TypeCode typeCode;
    struct LexToken {
        string content_p;
        int contentNum;
    } lexToken{};
    string analyzerResult;

    friend class GrammarAnalyzer;

public:
    static Lexer &getInstance(ifstream &inputFile, ofstream &outputFile);

    static string getTypeStr(TypeCode typeCode);

    void getNextToken();

    bool meetEOF();
};

#endif //SCANNER_LEXER_H
