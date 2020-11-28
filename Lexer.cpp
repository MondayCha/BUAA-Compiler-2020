//
// Created by Dlee on 2020/9/22.
//
#include "include/Lexer.h"

using namespace std;

static char lex_c;
static char lex_token[LEX_TOKEN_LEN];

Lexer::Lexer(ErrorHandle &errorHandle, ifstream &inputFile, ofstream &outputFile) : errorHandle(errorHandle),
                                                                                    inFile(inputFile),
                                                                                    outFile(outputFile) {
    lex_c = inFile.get();
    lineNum = 1;
    typeCode = WRONG;
    lexToken = {string(), 0};
}

Lexer &Lexer::getInstance(ErrorHandle &errorHandle, ifstream &inputFile, ofstream &outputFile) {
    static Lexer instance(errorHandle, inputFile, outputFile);
    return instance;
}

string Lexer::getTypeStr(TypeCode typeCode) {
    TypeCode firstCode = IDENFR;
    return typeString[typeCode - firstCode];
}

bool Lexer::meetEOF() {
    return typeCode == READEOF;
}

void Lexer::getNextToken() {
    lastLineNum = lineNum;
    typeCode = WRONG;
    int pt = 0;
    bool findError = false;
    // clean space and enter
    while (isspace(lex_c)) {
        if (lex_c == '\n') {
            ++lineNum;
        }
        lex_c = inFile.get();
    }
    // end of input
    if (lex_c == EOF) {
        typeCode = READEOF;
        return;
    }
    // variable or reverse word is begin with a-zA-Z and _
    if (isalund(lex_c)) {
        do {
            lex_token[pt++] = lex_c;
            lex_c = inFile.get();
        } while (isalund(lex_c) || isdigit(lex_c));
        lex_token[pt] = '\0';
        strToken = lex_token;
        string lowerToken = string();
        for (int i = 0; i < pt; ++i) {
            lex_token[i] = char(tolower(lex_token[i]));
        }
        lowerToken = lex_token;
        auto iter = reservedWordMap.find(lowerToken);
        if (iter != reservedWordMap.end()) {
            typeCode = iter->second;
            lexToken.content_p = lowerToken;
        } else {
            typeCode = IDENFR;  // reserved word
            lexToken.content_p = lowerToken;
        }
    }
        // int type 001 -100
    else if (isdigit(lex_c)) {
        lexToken.contentNum = 0;
        do {
            lex_token[pt++] = lex_c;
            lexToken.contentNum = 10 * lexToken.contentNum + (lex_c - '0');
            lex_c = inFile.get();
        } while (isdigit(lex_c));
        lex_token[pt] = '\0';
        strToken = lex_token;
        typeCode = INTCON;
    }
        // char type <'>
    else if (lex_c == '\'') {
        lex_c = inFile.get();
        // check if char variable is legal (not yet)
        // 字符中出现非法的符号
        if (errorHandle.checkIllegalCharReturnNull(lex_c, lineNum, findError)) {
            lex_c = inFile.get();
        } else {
            strToken = string(1, lex_c);
            lexToken.contentNum = (unsigned char) lex_c;
            typeCode = CHARCON;
            // check if char variable is one byte (not yet)
            lex_c = inFile.get();
            /*if (lex_c != '\'') {
                errorHandle.printErrorLine('a', lineNum, findError);
                do {
                    strToken = strToken.append(1, lex_c);
                    lex_c = inFile.get();
                } while (lex_c != '\'');
            }*/
            lex_c = inFile.get();
        }
    }
        // string <">
    else if (lex_c == '\"') {
        lex_c = inFile.get();
        // There are no symbols in the string
        if (lex_c == '\"') {
            // empty string 符号串中无任何符号
            errorHandle.printErrorLine('a', lineNum);
            strToken = "";
        } else {
            do {
                // 32,33,35-126
                if (!findError && !((32 <= lex_c && lex_c <= 33) || (35 <= lex_c && lex_c <= 126))) {
                    PRINT_ERR('a')  // illegal symbol appears
                }
                lex_token[pt++] = lex_c;
                if (lex_c == '\\') {
                    lex_token[pt++] = lex_c;
                }
                lex_c = inFile.get();
            } while (lex_c != '\"');
            lex_token[pt] = '\0';
            strToken = lex_token;
        }
        typeCode = STRCON;
        lex_c = inFile.get();
    }
        // operator
    else if (lex_c == '+' || lex_c == '-' || lex_c == '*' || lex_c == '/' || lex_c == ':' || lex_c == ';' ||
             lex_c == ','
             || lex_c == '(' || lex_c == ')' || lex_c == '[' || lex_c == ']' || lex_c == '{' || lex_c == '}') {
        strToken = string(1, lex_c);
        typeCode = operatorMap.find(strToken)->second;
        lex_c = inFile.get();
    }
        // <= >= == !=
    else if (lex_c == '<' || lex_c == '>' || lex_c == '=' || lex_c == '!') {
        lex_token[pt++] = lex_c;
        lex_c = inFile.get();
        if (lex_c == '=') {
            lex_token[pt++] = lex_c;
            lex_c = inFile.get();
        }
        lex_token[pt] = '\0';
        strToken = lex_token;
        typeCode = operatorMap.find(strToken)->second;
    } else {
        cerr << "read line failed!" << endl;
        typeCode = WRONG;
        return;
    }
    if (typeCode != WRONG) {
        // outFile << getTypeStr(typeCode) << " " << strToken << endl;
        analyzerResult = string(getTypeStr(typeCode).append(" ").append(strToken)); // .append("\n")
    }
}