//
// Created by Dlee on 2020/9/22.
//

// Prevent duplicate compilation
#ifndef SCANNER_TYPELIST_H
#define SCANNER_TYPELIST_H

#include <iostream>
#include <string>
#include <map>

using namespace std;

/////////////////////////////////////////////////
////////////////      IRCode    /////////////////
enum OperatorType {
    OpPLUS,
    OpMINU,
    OpMULT,
    OpDIV,
    OpASSIGN,
    OpScanf,
    OpPrint,
    OpConst,
    OpVar,
    OpFunc,
    OpJMain,
    OpExit,
    // 2020.11.24
    OpAssArray,
    OpGetArray,
    // 2020.11.26
    OpLSS,
    OpLEQ,
    OpGRE,
    OpGEQ,
    OpEQL,
    OpNEQ,
    OpBEZ,
    OpBNEZ,
    OpJmp,
    OpLabel,
    // 2020.11.27
    OpParam,
    OpPaVal,
    OpCall,
    OpReturn,
    OpRetVar,
};

const char *const operatorString[] = {
        "PLUS  ",
        "MINU  ",
        "MULT  ",
        "DIV   ",
        "ASSIGN",
        "Scanf ",
        "Print ",
        "ConDef",
        "VarDef",
        "FunDef",
        "JMain ",
        "Exit  ",
        "AssArr",
        "GetArr",
        "cd LSS",
        "cd LEQ",
        "cd GRE",
        "cd GEQ",
        "cd EQL",
        "cd NEQ",
        "bez   ",
        "bnez  ",
        "Jmp to",
        "Label:",
        "params",
        "call p",
        "Fc Cal",
        "Return",
        "retVar",
};

/////////////////////////////////////////////////
////////////////   SymbolTable  /////////////////
enum SymbolAtt {
    CON, VAR, FUN
};

enum SymbolType {
    CHAR, INT, VOID,
    MAIN,
    CHARVAR, INTVAR,
    INTTMP
};

/////////////////////////////////////////////////
////////////////     Lexer     //////////////////
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
    IFTK, PLUS, COLON, WRONG, READEOF
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