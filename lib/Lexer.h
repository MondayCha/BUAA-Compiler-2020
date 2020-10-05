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

#define TOKEN_LEN 233
// if use (isalpha(_C) | (_C == '_'))
// use of a signed integer operand with a binary bitwise operator
#define isalund(_C) (isalpha(_C) || (_C == '_'))

using namespace std;

class Lexer {
private:
    Lexer(ifstream &inputFile, ofstream &outputFile);   // Singleton Pattern
    ifstream &inFile;               // The copy function is protected, so track back
    ofstream &outFile;
public:
    static Lexer &getInstance(ifstream &inputFile, ofstream &outputFile);
    static string getTypeStr(TypeCode typeCode);
    bool getNextToken();
};

#endif //SCANNER_LEXER_H
