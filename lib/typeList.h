//
// Created by Dlee on 2020/9/22.
//

// Prevent duplicate compilation
#ifndef SCANNER_TYPELIST_H
#define SCANNER_TYPELIST_H

using namespace std;

enum TypeCode {
    IDENFR, ELSETK, MINU, ASSIGN,
    INTCON, SWITCHTK, MULT, SEMICN,
    CHARCON, CASETK, DIV, COMMA,
    STRCON, DEFAULTTK, LSS, LPARENT,
    CONSTTK, WHILETK, LEQ, RPARENT,
    INTTK, FORTK, GRE, LBRACK,
    CHARTK, SCANFTK, GEQ, RBRACK,
    VOIDTK, PRINTFTK, EQL, LBRACE,
    MAINTK, RETURNTK, NEQ, RBRACE,
    IFTK, PLUS, COLON, WRONG
};

const map<string, TypeCode> reservedWordMap = {
        {"const",   CONSTTK},
        {"int",     INTTK},
        {"char",    CHARTK},
        {"void",    VOIDTK},
        {"main",    MAINTK},
        {"if",      IFTK},
        {"else",    ELSETK},
        {"switch",  SWITCHTK},
        {"case",    CASETK},
        {"default", DEFAULTTK},
        {"while",   WHILETK},
        {"for",     FORTK},
        {"scanf",   SCANFTK},
        {"printf",  PRINTFTK},
        {"return",  RETURNTK},
};

const map<string, TypeCode> operatorMap = {
        {"+",  PLUS},
        {"-",  MINU},
        {"*",  MULT},
        {"/",  DIV},
        {"<",  LSS},
        {"<=", LEQ},
        {">",  GRE},
        {">=", GEQ},
        {"==", EQL},
        {"!=", NEQ},
        {":",  COLON},
        {"=",  ASSIGN},
        {";",  SEMICN},
        {",",  COMMA},
        {"(",  LPARENT},
        {")",  RPARENT},
        {"[",  LBRACK},
        {"]",  RBRACK},
        {"{",  LBRACE},
        {"}",  RBRACE}
};

const char *const typeString[] = {
        "IDENFR",
        "ELSETK",
        "MINU",
        "ASSIGN",
        "INTCON",
        "SWITCHTK",
        "MULT",
        "SEMICN",
        "CHARCON",
        "CASETK",
        "DIV",
        "COMMA",
        "STRCON",
        "DEFAULTTK",
        "LSS",
        "LPARENT",
        "CONSTTK",
        "WHILETK",
        "LEQ",
        "RPARENT",
        "INTTK",
        "FORTK",
        "GRE",
        "LBRACK",
        "CHARTK",
        "SCANFTK",
        "GEQ",
        "RBRACK",
        "VOIDTK",
        "PRINTFTK",
        "EQL",
        "LBRACE",
        "MAINTK",
        "RETURNTK",
        "NEQ",
        "RBRACE",
        "IFTK",
        "PLUS",
        "COLON"
};

#endif //SCANNER_TYPELIST_H
