//
// Created by Dlee on 2020/9/22.
//

#include <fstream>
#include <cctype>
#include "lib/Lexer.h"

using namespace std;

static char c;
static char token[TOKEN_LEN];
static int lineNum;             // Output the wrong line number

Lexer::Lexer(ifstream &inputFile, ofstream &outputFile) : inFile(inputFile), outFile(outputFile) {
    c = inFile.get();
    lineNum = 1;
}

Lexer &Lexer::getInstance(ifstream &inputFile, ofstream &outputFile) {
    static Lexer instance(inputFile, outputFile);
    return instance;
}

string Lexer::getTypeStr(TypeCode typeCode) {
    TypeCode firstCode = IDENFR;
    return typeString[typeCode - firstCode];
}

bool Lexer::getNextToken() {
    string strToken = string();
    TypeCode typeCode = WRONG;
    int pt = 0;
    // clean space and enter
    while (isspace(c)) {
        if (c == '\n') {
            ++lineNum;
        }
        c = inFile.get();
    }
    // end of input
    if (c == EOF) {
        return false;
    }
    // variable or reverse word is begin with a-zA-Z and _
    if (isalund(c)) {
        do {
            token[pt++] = c;
            c = inFile.get();
        } while (isalund(c) || isdigit(c));
        token[pt] = '\0';
        strToken = token;
        string lowerToken = string();
        for (int i = 0; i < pt; ++i) {
            token[i] = char(tolower(token[i]));
        }
        lowerToken = token;
        auto iter = reservedWordMap.find(lowerToken);
        if (iter != reservedWordMap.end()) {
            typeCode = iter->second;
        } else {
            typeCode = IDENFR;  // reserved word
        }
    }
        // int type 001 -100
    else if (isdigit(c)) {
        do {
            token[pt++] = c;
            c = inFile.get();
        } while (isdigit(c));
        token[pt] = '\0';
        strToken = token;
        typeCode = INTCON;
    }
        // char type <'>
    else if (c == '\'') {
        c = inFile.get();
        // check if char variable is legal (not yet)
        strToken = string(1, c);
        typeCode = CHARCON;
        // check if char variable is one byte (not yet)
        c = inFile.get();
        c = inFile.get();
    }
        // string <">
    else if (c == '\"') {
        c = inFile.get();
        do {
            token[pt++] = c;
            c = inFile.get();
        } while (c != '\"');
        token[pt] = '\0';
        strToken = token;
        typeCode = STRCON;
        c = inFile.get();
    }
        // operator
    else if (c == '+' || c == '-' || c == '*' || c == '/' || c == ':' || c == ';' || c == ','
             || c == '(' || c == ')' || c == '[' || c == ']' || c == '{' || c == '}') {
        strToken = string(1, c);
        typeCode = operatorMap.find(strToken)->second;
        c = inFile.get();
    }
        // <= >= == !=
    else if (c == '<' || c == '>' || c == '=' || c == '!') {
        token[pt++] = c;
        c = inFile.get();
        if (c == '=') {
            token[pt++] = c;
            c = inFile.get();
        }
        token[pt] = '\0';
        strToken = token;
        typeCode = operatorMap.find(strToken)->second;
    } else {
        cerr << "read line failed!" << endl;
        return false;
    }
    if (typeCode != WRONG) {
        outFile << getTypeStr(typeCode) << " " << strToken << endl;
    }
    return true;
}