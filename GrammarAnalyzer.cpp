//
// Created by Dlee on 2020/10/5.
//

#include "include/GrammarAnalyzer.h"

GrammarAnalyzer::GrammarAnalyzer(Lexer &pronLexer, SymbolTable &pronSymbol, std::ofstream &pronFile)
        : lexer(pronLexer), symbolTable(pronSymbol), outFile(pronFile) {
    p_content = 0;
}

GrammarAnalyzer &GrammarAnalyzer::getInstance(Lexer &pronLexer, SymbolTable &pronSymbol, std::ofstream &pronOutFile) {
    static GrammarAnalyzer instance(pronLexer, pronSymbol, pronOutFile);
    return instance;
}

void GrammarAnalyzer::analyzeGrammar() {
    GET_TOKEN;
    programAnalyzer();
}

/*＜程序＞::= ［＜常量说明＞］［＜变量说明＞］{＜有返回值函数定义＞|＜无返回值函数定义＞}＜主函数＞*/
void GrammarAnalyzer::programAnalyzer() {
    // 常量说明
    if (TOKEN_TYPE == CONSTTK) {
        constDeclarationAnalyzer();
    }
    // 变量说明或有返回值函数定义
    if (TYPE_IDEN) {
        varDeclarationAnalyzer();
    }
    // {＜有返回值函数定义＞|＜无返回值函数定义＞}
}

/*＜常量说明＞ ::=  const＜常量定义＞;{ const＜常量定义＞;} */
void GrammarAnalyzer::constDeclarationAnalyzer() {
    // TOKEN_TYPE == CONSTTK
    do {
        PRINT_GET;
        // ＜常量定义＞
        constDefinitionAnalyzer();
        // ;
        if (TOKEN_TYPE == SEMICN) {
            PRINT_GET;
        }
    } while (TOKEN_TYPE == CONSTTK);
    outFile << "<常量说明>" << endl;
}

/*＜常量定义＞ ::=   int＜标识符＞＝＜整数＞{,＜标识符＞＝＜整数＞}
| char＜标识符＞＝＜字符＞{,＜标识符＞＝＜字符＞} */
void GrammarAnalyzer::constDefinitionAnalyzer() {
    // int
    if (TOKEN_TYPE == INTTK) {
        PRINT_GET;
        // ＜标识符＞＝＜整数＞
        intAssignAnalyzer();
        while (TOKEN_TYPE == COMMA) {
            PRINT_GET;
            intAssignAnalyzer();
        }
    }
        // char
    else if (TOKEN_TYPE == CHARTK) {
        PRINT_GET;
        // ＜标识符＞＝＜字符＞
        charAssignAnalyzer();
        while (TOKEN_TYPE == COMMA) {
            PRINT_GET;
            charAssignAnalyzer();
        }
    }
    outFile << "<常量定义>" << endl;
}

/*＜标识符＞＝＜整数＞*/
void GrammarAnalyzer::intAssignAnalyzer() {
    // ＜标识符＞
    if (TOKEN_TYPE == IDENFR) {
        PRINT_GET;
        // ＝
        if (TOKEN_TYPE == ASSIGN) {
            PRINT_GET;
            // ＜整数＞
            integerAnalyzer();
        }
    }
}

/*＜整数＞::= ［＋｜－］＜无符号整数＞*/
void GrammarAnalyzer::integerAnalyzer() {
    bool symbol = true;
    if (TOKEN_TYPE == MINU || TOKEN_TYPE == PLUS) {
        symbol = (TOKEN_TYPE == MINU) ? false : true;
        PRINT_GET;
    }
    unsignedIntAnalyzer();
    outFile << "<整数>" << endl;
}

/*＜无符号整数＞  ::= ＜数字＞｛＜数字＞｝*/
int GrammarAnalyzer::unsignedIntAnalyzer() {
    if (TOKEN_TYPE == INTCON) {
        outFile << lexer.analyzerResult;
        int returnAns = lexer.lexToken.contentNum;
        lexer.getNextToken();
        outFile << "<无符号整数>" << endl;
        return returnAns;
    } else { ;//error
    }
    return -1;
}

/*＜标识符＞＝＜字符＞*/
void GrammarAnalyzer::charAssignAnalyzer() {
    // ＜标识符＞
    if (TOKEN_TYPE == IDENFR) {
        PRINT_GET;
        // ＝
        if (TOKEN_TYPE == ASSIGN) {
            PRINT_GET;
            // ＜字符＞
            characterAnalyzer();
        }
    }
}

/*＜字符＞*/
void GrammarAnalyzer::characterAnalyzer() {
    if (TOKEN_TYPE == CHARCON) {
        PRINT_GET;
    }
}

/*＜变量说明＞  ::= ＜变量定义＞;{＜变量定义＞;}
 * 需要判断第一次遇到有返回值函数的情况*/
void GrammarAnalyzer::varDeclarationAnalyzer() {
    // 变量定义
    string tmp_identity, tmp_output1, tmp_output2, tmp_output3;
    TypeCode tmp_typeCode = WRONG;
    bool meetFunc = false;
    do {
        // ＜变量定义无初始化＞|＜变量定义及初始化＞|＜有返回值函数定义＞
        // ＜类型标识符＞
        tmp_output1 = lexer.analyzerResult;
        tmp_typeCode = lexer.typeCode;
        lexer.getNextToken();
        // ＜标识符＞
        tmp_output2 = lexer.analyzerResult;
        tmp_identity = lexer.lexToken.content_p;
        lexer.getNextToken();
        // ＜有返回值函数定义＞
        if (TOKEN_TYPE == LPARENT) {
            outFile << "<变量说明>" << endl;
            outFile << tmp_output1;
            outFile << tmp_output2;
            // cout << "func jump " << tmp_identity + "\n";
            symbolTable.addFunc(tmp_identity, true);
            meetFunc = true;
            speReValFuncDefAnalyzer();
        } else {
            // ＜变量定义＞
            outFile << tmp_output1;
            outFile << tmp_output2;
            tmp_output3 = varDefinitionAnalyzer(tmp_typeCode);
            // ;
            if (TOKEN_TYPE == SEMICN) {
                outFile << tmp_output3;
                outFile << "<变量定义>" << endl;
                PRINT_GET;
            }
        }
        if (meetFunc) {
            return;
        }
    } while (TYPE_IDEN);
    outFile << "<变量说明>" << endl;
}

/*＜变量说明＞  ::= ＜变量定义＞;{＜变量定义＞;}
 * 不需要判断第一次遇到有返回值函数的情况*/
void GrammarAnalyzer::varDeclareNoFuncAnalyzer() {
    // 变量定义
    string tmp_identity, tmp_output3;
    TypeCode tmp_typeCode = WRONG;
    do {
        // ＜变量定义无初始化＞|＜变量定义及初始化＞
        // ＜类型标识符＞
        outFile << lexer.analyzerResult;
        tmp_typeCode = lexer.typeCode;
        lexer.getNextToken();
        // ＜标识符＞
        outFile << lexer.analyzerResult;
        // tmp_identity = lexer.lexToken.content_p;
        lexer.getNextToken();
        // ＜变量定义＞
        tmp_output3 = varDefinitionAnalyzer(tmp_typeCode);
        // ;
        if (TOKEN_TYPE == SEMICN) {
            outFile << tmp_output3;
            outFile << "<变量定义>" << endl;
            PRINT_GET;
        }
    } while (TYPE_IDEN);
    outFile << "<变量说明>" << endl;
}

string GrammarAnalyzer::varDefinitionAnalyzer(TypeCode typeCode) {
    // ＜类型标识符＞＜标识符＞=＜常量＞
    // ＜标识符＞['['＜无符号整数＞']'|'['＜无符号整数＞']''['＜无符号整数＞']']
    int level = 0, length1 = 0, length2 = 0;
    if (TOKEN_TYPE == LBRACK) {
        arrayVarAnalyzer(level, length1, length2);
    }
    if (TOKEN_TYPE == ASSIGN) {
        // 变量定义及初始化
        PRINT_GET;
        // 初始化
        varInitAnalyzer(level, length1, length2, (typeCode == INTTK));
        return "<变量定义及初始化>\n";
    } else if (TOKEN_TYPE == COMMA) {
        // 变量定义无初始化
        do {
            PRINT_GET;
            PRINT_GET;
            if (TOKEN_TYPE == LBRACK) {
                arrayVarAnalyzer();
            }
        } while (TOKEN_TYPE == COMMA);
        return "<变量定义无初始化>\n";
    }
    return "<变量定义无初始化>\n";
}

void GrammarAnalyzer::reValFuncDefAnalyzer() {

}

/*特殊的有返回值函数*/
/*＜有返回值函数定义＞  ::=  ＜声明头部＞'('＜参数表＞')' '{'＜复合语句＞'}'*/
void GrammarAnalyzer::speReValFuncDefAnalyzer() {
    outFile << "<声明头部>" << endl;
    // '('
    CHECK_GET(LPARENT, "speReValFuncDefAnalyzer()");
    // ＜参数表＞')'
    parameterTableAnalyzer();
    // '{'
    CHECK_GET(LBRACE, "speReValFuncDefAnalyzer()");
    //＜复合语句＞'}'
    compoundStatementAnalyzer();
    //'}'
    CHECK_GET(RBRACE, "speReValFuncDefAnalyzer()");
}

/*'['＜无符号整数＞']'|'['＜无符号整数＞']''['＜无符号整数＞']'
 * return level*/
void GrammarAnalyzer::arrayVarAnalyzer(int &level, int &l1, int &l2) {
    PRINT_GET;
    l1 = unsignedIntAnalyzer();
    if (TOKEN_TYPE != RBRACK) { ;
    }
    PRINT_GET;
    if (TOKEN_TYPE == LBRACK) {
        PRINT_GET;
        l2 = unsignedIntAnalyzer();
        if (TOKEN_TYPE != RBRACK) { ;
        }
        PRINT_GET;
        level = 2;
        return;
    }
    level = 1;
}

void GrammarAnalyzer::arrayVarAnalyzer() {
    PRINT_GET;
    unsignedIntAnalyzer();
    if (TOKEN_TYPE != RBRACK) { ;
    }
    PRINT_GET;
    if (TOKEN_TYPE == LBRACK) {
        PRINT_GET;
        unsignedIntAnalyzer();
        if (TOKEN_TYPE != RBRACK) { ;
        }
        PRINT_GET;
    }
}

/*初始化[][]*/
void GrammarAnalyzer::varInitAnalyzer(int level, int length1, int length2, bool isInteger) {
    if (level == 0) {
        constantAnalyzer(isInteger);
    } else if (level == 1) {
        if (TOKEN_TYPE != LBRACK) { ;//error
        }
        PRINT_GET;
        for (int i = 0; i < length1 - 1; i++) {
            constantAnalyzer(isInteger);
            if (TOKEN_TYPE != COMMA) { ;//error
            }
            PRINT_GET;
        }
        constantAnalyzer(isInteger);
        if (TOKEN_TYPE != RBRACK) { ;//error
        }
        PRINT_GET;
    } else {
        // 外层大括号
        if (TOKEN_TYPE != LBRACK) { ;//error
        }
        PRINT_GET;
        // 读取{}，
        for (int i = 0; i < length1 - 1; i++) {
            varInitAnalyzer(1, length2, length2, isInteger);
            if (TOKEN_TYPE != COMMA) { ;//error
            }
            PRINT_GET;
        }
        varInitAnalyzer(1, length2, length2, isInteger);
        if (TOKEN_TYPE != RBRACK) { ;//error
        }
        PRINT_GET;
    }
}

/*＜常量＞   ::=  ＜整数＞|＜字符＞*/
void GrammarAnalyzer::constantAnalyzer(bool isInteger) {
    if (isInteger) {
        integerAnalyzer();
    } else {
        if (TOKEN_TYPE != CHARTK) { ;//error
        }
        PRINT_GET;
    }
    outFile << "<常量>" << endl;
}

/*＜参数表＞')'*/
void GrammarAnalyzer::parameterTableAnalyzer() {
    if (TYPE_IDEN) {
        // ＜类型标识符＞＜标识符＞
        paraTableEleAnalyzer();
    }
    while (TOKEN_TYPE == COMMA) {
        PRINT_GET;
        // ＜类型标识符＞＜标识符＞
        paraTableEleAnalyzer();
    }
    if (TOKEN_TYPE == RPARENT) {
        outFile << "<参数表>" << endl;
        PRINT_GET;
    } else {
        cerr << "error in para";
    }
}

/*＜类型标识符＞＜标识符＞*/
void GrammarAnalyzer::paraTableEleAnalyzer() {
    if (!TYPE_IDEN) {
        cerr << "paraTableEleAnalyzer()";
    }
    PRINT_GET;
    if (TOKEN_TYPE != IDENFR) {
        cerr << "paraTableEleAnalyzer()";
    }
    PRINT_GET;
}

/*＜复合语句＞   ::=  ［＜常量说明＞］［＜变量说明＞］＜语句列＞*/
void GrammarAnalyzer::compoundStatementAnalyzer() {
    // 常量说明
    if (TOKEN_TYPE == CONSTTK) {
        constDeclarationAnalyzer();
    }
    // 变量说明
    if (TYPE_IDEN) {
        varDeclareNoFuncAnalyzer();
    }
    // ＜语句列＞
    statementColumnAnalyzer();
    outFile << "<复合语句>" << endl;
}

/*＜语句列＞   ::= ｛＜语句＞｝*/
void GrammarAnalyzer::statementColumnAnalyzer() {
    while (statementAnalyzer()) { ;
    }
    if (TOKEN_TYPE == RBRACE) {
        outFile << "<语句列>" << endl;
    }
}

/*＜语句＞    ::= ＜循环语句＞｜＜条件语句＞| ＜有返回值函数调用语句＞;
 * |＜无返回值函数调用语句＞;｜＜赋值语句＞;｜＜读语句＞;
 * ｜＜写语句＞;｜＜情况语句＞｜＜空＞;|＜返回语句＞; | '{'＜语句列＞'}'*/
bool GrammarAnalyzer::statementAnalyzer() {
    if (TOKEN_TYPE == RBRACE) {
        return false;
    }
    // '{'＜语句列＞'}'
    if (TOKEN_TYPE == LBRACE) {
        PRINT_GET;
        statementColumnAnalyzer();
        CHECK_GET(RBRACE, "statementAnalyzer()");
    }
        // ＜循环语句＞
    else if (TOKEN_TYPE == WHILETK || TOKEN_TYPE == FORTK) {
        loopStatementAnalyzer();
    }
        // ＜条件语句＞|
    else if () {

    }
        // ＜有返回值函数调用语句＞;
    else if () {

    }
        // ＜无返回值函数调用语句＞;
    else if () {

    }
        // ＜赋值语句＞;
    else if () {

    }
        // ＜读语句＞;
    else if () {

    }
        // ＜写语句＞;
    else if () {

    }
        // ＜情况语句＞
    else if () {

    }
        // ＜空＞;
    else if (TOKEN_TYPE == SEMICN) {
        PRINT_GET;
    }
        // ＜返回语句＞;
    else if () {

    } else {
        return false;
    }
    outFile << "<语句>" << endl;
    return true;
}

/*＜循环语句＞   ::=  while '('＜条件＞')'＜语句＞|
 * for'('＜标识符＞＝＜表达式＞;＜条件＞;＜标识符＞＝＜标识符＞(+|-)＜步长＞')'＜语句＞ */
void GrammarAnalyzer::loopStatementAnalyzer() {
    if (TOKEN_TYPE == WHILETK) {
        PRINT_GET;
        CHECK_GET(LPARENT, "whileStatementAnalyzer()");
        conditionAnalyzer();
        CHECK_GET(RPARENT, "whileStatementAnalyzer()");
        statementAnalyzer();
    } else if (TOKEN_TYPE == FORTK) {
        //for
        PRINT_GET;
        //'('
        CHECK_GET(LPARENT, "forStatementAnalyzer()");
        //＜标识符＞
        CHECK_GET(IDENFR, "forStatementAnalyzer()");
        //＝
        CHECK_GET(ASSIGN, "forStatementAnalyzer()");
        //for'('＜标识符＞＝＜表达式＞
        expressionAnalyzer();
        //;
        CHECK_GET(SEMICN, "forStatementAnalyzer()");
        //＜标识符＞
        CHECK_GET(IDENFR, "forStatementAnalyzer()");
        //＝
        CHECK_GET(ASSIGN, "forStatementAnalyzer()");
        //＜标识符＞
        CHECK_GET(IDENFR, "forStatementAnalyzer()");
        //(+|-)
        if (TYPE_ADDE) {
            PRINT_GET;
        } else {
            cerr << "for";
        }
        //＜步长＞
        unsignedIntAnalyzer();
        // ')'
        CHECK_GET(RPARENT, "forStatementAnalyzer()");
        // ＜语句＞
        statementAnalyzer();
    }
    outFile << "<循环语句>" << endl;
}

/*＜条件＞::=  ＜表达式＞＜关系运算符＞＜表达式＞*/
void GrammarAnalyzer::conditionAnalyzer() {
    expressionAnalyzer();
    /*＜关系运算符＞  ::=  <｜<=｜>｜>=｜!=｜==*/
    if (TOKEN_TYPE == LSS || TOKEN_TYPE == LEQ || TOKEN_TYPE == GRE ||
        TOKEN_TYPE == GEQ || TOKEN_TYPE == EQL || TOKEN_TYPE == NEQ) {
        PRINT_GET;
    } else {
        cerr << "relationalOperatorAnalyzer()";
    }
    expressionAnalyzer();
    outFile << "<条件>" << endl;
}

/*＜表达式＞    ::= ［＋｜－］＜项＞{＜加法运算符＞＜项＞}*/
void GrammarAnalyzer::expressionAnalyzer() {
    if (TYPE_ADDE) {
        PRINT_GET;
    }
    itemAnalyzer();
    while (TYPE_ADDE) {
        PRINT_GET;
        itemAnalyzer();
    }
    outFile << "<表达式>" << endl;
}

void GrammarAnalyzer::itemAnalyzer() {

}

