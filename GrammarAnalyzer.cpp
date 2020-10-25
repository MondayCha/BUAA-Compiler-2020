//
// Created by Dlee on 2020/10/5.
//

#include "include/GrammarAnalyzer.h"

SymbolType grammar_func_with_return;
bool grammar_state_with_return;

GrammarAnalyzer::GrammarAnalyzer(Lexer &pronLexer, SymbolTable &pronSymbol, ErrorHandle &pronError,
                                 std::ofstream &pronFile)
        : lexer(pronLexer), symbolTable(pronSymbol), errorHandle(pronError), outFile(pronFile) {
    grammar_func_with_return = VOID;
    grammar_state_with_return = false;
}

GrammarAnalyzer &GrammarAnalyzer::getInstance(Lexer &pronLexer, SymbolTable &pronSymbol, ErrorHandle &pronError,
                                              std::ofstream &pronOutFile) {
    static GrammarAnalyzer instance(pronLexer, pronSymbol, pronError, pronOutFile);
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
    } else {
        symbolTable.endGlobalIdenSymbol();
    }
/*    auto iter = symbolTable.globalIdenTable.begin();
    for(iter; iter != symbolTable.globalIdenTable.end();iter++){
        cout << "iter is " << iter->second.name << endl;
        cout << "iter type is " << iter->second.symbolType << endl;
    }*/
    // {＜有返回值函数定义＞|＜无返回值函数定义＞}＜主函数＞
    functionDeclarationAnalyzer();
    PRINT_MES(("<程序>"))
}

/*＜常量说明＞ ::=  const＜常量定义＞;{ const＜常量定义＞;} */
void GrammarAnalyzer::constDeclarationAnalyzer() {
    // TOKEN_TYPE == CONSTTK
    do {
        PRINT_GET
        // ＜常量定义＞
        constDefinitionAnalyzer();
        // ;
        CHECK_SEMICN
    } while (TOKEN_TYPE == CONSTTK);
    PRINT_MES(("<常量说明>"))
}

/*＜常量定义＞ ::=   int＜标识符＞＝＜整数＞{,＜标识符＞＝＜整数＞}
| char＜标识符＞＝＜字符＞{,＜标识符＞＝＜字符＞} */
void GrammarAnalyzer::constDefinitionAnalyzer() {
    // int
    if (TOKEN_TYPE == INTTK) {
        PRINT_GET
        // ＜标识符＞＝＜整数＞
        intAssignAnalyzer();
        while (TOKEN_TYPE == COMMA) {
            PRINT_GET
            intAssignAnalyzer();
        }
    }
        // char
    else if (TOKEN_TYPE == CHARTK) {
        PRINT_GET
        // ＜标识符＞＝＜字符＞
        charAssignAnalyzer();
        while (TOKEN_TYPE == COMMA) {
            PRINT_GET
            charAssignAnalyzer();
        }
    }
    PRINT_MES(("<常量定义>"))
}

/*＜整数＞::= ［＋｜－］＜无符号整数＞*/
int GrammarAnalyzer::integerAnalyzer(bool &checkBit) {
    bool symbol = true;
    if (TOKEN_TYPE == MINU || TOKEN_TYPE == PLUS) {
        symbol = !(TOKEN_TYPE == MINU);
        PRINT_GET
    }
    int ans = unsignedIntAnalyzer();
    if (ans == -1) {
        checkBit = false;
    } else {
        checkBit = true;
    }
    PRINT_MES(("<整数>"))
    return symbol ? ans : 0 - ans;
}

int GrammarAnalyzer::integerAnalyzer() {
    bool noCheck = false;
    return integerAnalyzer(noCheck);
}

/*＜无符号整数＞  ::= ＜数字＞｛＜数字＞｝*/
int GrammarAnalyzer::unsignedIntAnalyzer() {
    if (TOKEN_TYPE == INTCON) {
        PRINT_MES(lexer.analyzerResult)
        int returnAns = lexer.lexToken.contentNum;
        lexer.getNextToken();
        PRINT_MES(("<无符号整数>"))
        return returnAns;
    }
    return -1;
}

/*＜标识符＞＝＜整数＞*/
void GrammarAnalyzer::intAssignAnalyzer() {
    string name;
    string lowerName;
    int content;
    // ＜标识符＞
    if (TOKEN_TYPE == IDENFR) {
        name = lexer.strToken;
        lowerName = lexer.lexToken.content_p;
        // cout << "address1 is " << &lexer.strToken << " a2 is " << &name << " value is " << name << endl;
        PRINT_GET
        // ＝
        if (TOKEN_TYPE == ASSIGN) {
            PRINT_GET
            // <常量>类型不一致
            if (TOKEN_TYPE == CHARCON) {
                // PRINT_G_ERR('o')
                content = characterAnalyzer();
            } else {
                // ＜整数＞
                content = integerAnalyzer();
            }
            if (!errorHandle.checkDupIdenDefine(lowerName, lexer.lineNum)) {
                symbolTable.insertSymbolToLocal(*(new ConSym(name, lowerName, INT, content)));
            }
        }
    }
}

/*＜标识符＞＝＜字符＞*/
void GrammarAnalyzer::charAssignAnalyzer() {
    string name;
    string lowerName;
    int content;
    // ＜标识符＞
    if (TOKEN_TYPE == IDENFR) {
        name = lexer.strToken;
        lowerName = lexer.lexToken.content_p;
        PRINT_GET
        // ＝
        if (TOKEN_TYPE == ASSIGN) {
            PRINT_GET
            // ＜字符＞
            if (TOKEN_TYPE == CHARCON) {
                content = characterAnalyzer();
            } else {
                // ＜整数＞
                // PRINT_G_ERR('o')
                content = integerAnalyzer();
            }
            content = characterAnalyzer();
            if (!errorHandle.checkDupIdenDefine(lowerName, lexer.lineNum)) {
                symbolTable.insertSymbolToLocal(*(new ConSym(name, lowerName, CHAR, content)));
            }
        }
    }
}

/*＜字符＞*/
int GrammarAnalyzer::characterAnalyzer() {
    if (TOKEN_TYPE == CHARCON) {
        int ans = lexer.lexToken.contentNum;
        PRINT_GET
        return ans;
    }
    return -1;
}

/*＜变量说明＞  ::= ＜变量定义＞;{＜变量定义＞;}
 * 需要判断第一次遇到有返回值函数的情况*/
void GrammarAnalyzer::varDeclarationAnalyzer() {
    // 变量定义
    string tmp_name, tmp_lowerName, tmp_output1, tmp_output2;
    TypeCode tmp_typeCode = WRONG;
    bool meetFunc = false;
    int loopTime = 0;
    do {
        // ＜变量定义无初始化＞|＜变量定义及初始化＞|＜有返回值函数定义＞
        // ＜类型标识符＞
        tmp_output1 = lexer.analyzerResult;
        tmp_typeCode = lexer.typeCode;
        lexer.getNextToken();
        // ＜标识符＞
        tmp_output2 = lexer.analyzerResult;
        tmp_name = lexer.strToken;
        tmp_lowerName = lexer.lexToken.content_p;
        lexer.getNextToken();
        // ＜有返回值函数定义＞
        if (TOKEN_TYPE == LPARENT) {
            symbolTable.endGlobalIdenSymbol();
            if (loopTime != 0) {
                PRINT_MES(("<变量说明>"))
            }
            PRINT_MES(tmp_output1)
            PRINT_MES(tmp_output2)
            // cout << "func jump " << tmp_name + "\n";
            meetFunc = true;
            speReValFuncDefAnalyzer(tmp_name, tmp_lowerName, tmp_typeCode);
        } else {
            // ＜变量定义＞
            PRINT_MES(tmp_output1)
            PRINT_MES(tmp_output2)
            varDefinitionAnalyzer(tmp_name, tmp_lowerName, tmp_typeCode);
            PRINT_MES(("<变量定义>"))
            // ;
            CHECK_SEMICN
        }
        if (meetFunc) {
            return;
        }
        ++loopTime;
    } while (TYPE_IDEN);
    PRINT_MES("<变量说明>")
    symbolTable.endGlobalIdenSymbol();
}

/*＜变量说明＞  ::= ＜变量定义＞;{＜变量定义＞;}
 * 不需要判断第一次遇到有返回值函数的情况*/
void GrammarAnalyzer::varDeclareNoFuncAnalyzer() {
    // 变量定义
    string tmp_name, tmp_lowerName;
    TypeCode tmp_typeCode = WRONG;
    do {
        // ＜变量定义无初始化＞|＜变量定义及初始化＞
        // ＜类型标识符＞
        PRINT_MES(lexer.analyzerResult)
        tmp_typeCode = lexer.typeCode;
        lexer.getNextToken();
        // ＜标识符＞
        PRINT_MES(lexer.analyzerResult)
        // tmp_identity = lexer.strToken;
        tmp_name = LEX_NAME;
        tmp_lowerName = LEX_LONA;
        lexer.getNextToken();
        // ＜变量定义＞
        varDefinitionAnalyzer(tmp_name, tmp_lowerName, tmp_typeCode);
        // ;
        PRINT_MES(("<变量定义>"))
        CHECK_SEMICN
    } while (TYPE_IDEN);
    PRINT_MES(("<变量说明>"))
}

void GrammarAnalyzer::varDefinitionAnalyzer(string &name, string &lowerName, TypeCode typeCode) {
    // ＜类型标识符＞＜标识符＞=＜常量＞
    // ＜标识符＞['['＜无符号整数＞']'|'['＜无符号整数＞']''['＜无符号整数＞']']
    int level = 0, length1 = 0, length2 = 0;
    SymbolType symType = symbolTable.convertTypeCode(typeCode);
    if (TOKEN_TYPE == LBRACK) {
        arrayVarAnalyzer(level, length1, length2);
    }
    VarSym varSym = *(new VarSym(name, lowerName, symType, level, length1, length2));
    if (!errorHandle.checkDupIdenDefine(lowerName, LEX_LINE)) {
        symbolTable.insertSymbolToLocal(varSym);
        // cout << "symbolTable insert Symbol To Local named " << varSym.name << " type is " << varSym.symbolType << endl;
    }
    if (TOKEN_TYPE == ASSIGN) {
        // 变量定义及初始化
        PRINT_GET
        // 初始化
        varInitAnalyzer(level, length1, length2, (typeCode == INTTK));
        PRINT_MES("<变量定义及初始化>")
        return;
    } else if (TOKEN_TYPE == COMMA) {
        // 变量定义无初始化
        string subName;
        string subLowerName;
        do {
            // ,
            PRINT_GET
            // iden
            subName = LEX_NAME;
            subLowerName = LEX_LONA;
            level = 0, length1 = 0, length2 = 0;
            PRINT_GET
            if (TOKEN_TYPE == LBRACK) {
                arrayVarAnalyzer(level, length1, length2);
            }
            if (!errorHandle.checkDupIdenDefine(subLowerName, LEX_LINE)) {
                symbolTable.insertSymbolToLocal(
                        *(new VarSym(subName, subLowerName, symType, level, length1, length2)));
            }
        } while (TOKEN_TYPE == COMMA);
    }
    PRINT_MES("<变量定义无初始化>")
}

/*特殊的有返回值函数*/
/*＜有返回值函数定义＞  ::=  ＜声明头部＞'('＜参数表＞')' '{'＜复合语句＞'}'*/
void GrammarAnalyzer::speReValFuncDefAnalyzer(string &name, string &lowerName, TypeCode tk) {
    PRINT_MES(("<声明头部>"))
    // '('
    CHECK_GET(LPARENT, "speReValFuncDefAnalyzer()");
    // ＜参数表＞
    FuncSym funcSym = *(new FuncSym(name, lowerName, symbolTable.convertTypeCode(tk)));
    parameterTableAnalyzer(funcSym, true);
    // ')'
    if (!errorHandle.checkDupFuncDefine(lowerName, LEX_LINE)) {
        symbolTable.insertFuncToGlobal(funcSym);
    }
    // 有参数函数定义
    CHECK_RPARENT
    // '{'
    CHECK_GET(LBRACE, "speReValFuncDefAnalyzer {");
    //＜复合语句＞
    grammar_func_with_return = symbolTable.convertTypeCode(tk);
    // cerr << "grammar_func_with_return " << grammar_func_with_return << endl;
    compoundStatementAnalyzer();
    //'}'
    if (!grammar_state_with_return) {
        errorHandle.printErrorOfReturnMatch(LEX_LINE);
    }
    CHECK_GET(RBRACE, "speReValFuncDefAnalyzer }");
    PRINT_MES(("<有返回值函数定义>"))
    symbolTable.endLocalIdenSymbol();
    grammar_func_with_return = VOID;
    grammar_state_with_return = false;
}

/*'['＜无符号整数＞']'|'['＜无符号整数＞']''['＜无符号整数＞']'
 * return level*/
void GrammarAnalyzer::arrayVarAnalyzer(int &level, int &l1, int &l2) {
    PRINT_GET
    l1 = unsignedIntAnalyzer();
    CHECK_RBRACK
    if (TOKEN_TYPE == LBRACK) {
        PRINT_GET
        l2 = unsignedIntAnalyzer();
        CHECK_RBRACK
        level = 2;
        return;
    }
    level = 1;
}

void GrammarAnalyzer::arrayVarAnalyzer() {
    PRINT_GET
    unsignedIntAnalyzer();
    if (TOKEN_TYPE != RBRACK) { ;
    }
    PRINT_GET
    if (TOKEN_TYPE == LBRACK) {
        PRINT_GET
        unsignedIntAnalyzer();
        if (TOKEN_TYPE != RBRACK) { ;
        }
        PRINT_GET
    }
}

/*初始化[][] {}{}*/
void GrammarAnalyzer::varInitAnalyzer(int level, int length1, int length2, bool isInteger) {
    if (level == 0) {
        // 不会多一维的
        constInVarInitAnalyzer_return_error(isInteger);
    } else if (level == 1) {
        // 一维数组
        PRINT_GET
        for (int i = 0; i < length1 - 1; i++) {
            constInVarInitAnalyzer_return_error(isInteger);
            if (TOKEN_TYPE != COMMA) { ;//error
                // 遇到了提前结束的情况
                // 如：int a[4] = {0,1}
                readToRightBrack();
                return;
            }
            PRINT_GET
        }
        constInVarInitAnalyzer_return_error(isInteger);
        if (TOKEN_TYPE != RBRACE) { ;//error
            readToRightBrack();
            return;
        }
        PRINT_GET
    } else {
        // 外层大括号
        PRINT_GET
        // 读取{}，
        for (int i = 0; i < length1 - 1; i++) {
            varInitAnalyzer(1, length2, length2, isInteger);
            if (TOKEN_TYPE != COMMA) { ;//error
                readToRightBrack();
                return;
            }
            PRINT_GET
        }
        varInitAnalyzer(1, length2, length2, isInteger);
        if (TOKEN_TYPE == RBRACE) {
            PRINT_GET
            return;
        }
        while (TOKEN_TYPE != RBRACE) { ;//error
            if (TOKEN_TYPE == COMMA) {
                PRINT_GET
            }
            varInitAnalyzer(1, length2, length2, isInteger);
        }
        readToRightBrack();
    }
}

/*＜常量＞   ::=  ＜整数＞|＜字符＞*/
bool GrammarAnalyzer::constInVarInitAnalyzer_return_error(bool isInteger) {
    if (isInteger) {
        if (TOKEN_TYPE == CHARCON) {
            errorHandle.printErrorOfUnMatchConst(LEX_LINE);
            PRINT_GET
        } else {
            integerAnalyzer();
        }
    } else {
        if (TOKEN_TYPE != CHARCON) {
            //is int
            errorHandle.printErrorOfUnMatchConst(LEX_LINE);
            integerAnalyzer();
        } else {
            PRINT_GET
        }
    }
    PRINT_MES(("<常量>"))
    return false;
}

SymbolType GrammarAnalyzer::constantAnalyzer() {
    SymbolType symbolType = VOID;
    if (TOKEN_TYPE == CHARCON) {
        PRINT_GET
        symbolType = CHAR;
    } else {
        integerAnalyzer();
        symbolType = INT;
    }
    PRINT_MES(("<常量>"))
    return symbolType;
}

/*＜参数表＞')'*/
void GrammarAnalyzer::parameterTableAnalyzer(FuncSym &funcSym, bool declaration) {
    if (TYPE_IDEN) {
        // ＜类型标识符＞＜标识符＞
        paraTableEleAnalyzer(funcSym, declaration);
    }
    while (TOKEN_TYPE == COMMA) {
        PRINT_GET
        // ＜类型标识符＞＜标识符＞
        paraTableEleAnalyzer(funcSym, declaration);
    }
    /*for (int i = 0; i < funcSym.parameters.max_size(); i++) {
        cout << "pa is " << funcSym.parameters.at(i).name << endl;
    }*/
    PRINT_MES(("<参数表>"))
    /*if (TOKEN_TYPE == RPARENT) {

    } else {
        cerr << "error in para";
    }*/
}

/*＜类型标识符＞＜标识符＞*/
void GrammarAnalyzer::paraTableEleAnalyzer(FuncSym &funcSym, bool declaration) {
    if (!TYPE_IDEN) {
        cerr << "paraTableEleAnalyzer()";
    }
    TypeCode tmp_type = lexer.typeCode;
    PRINT_GET
    if (TOKEN_TYPE != IDENFR) {
        cerr << "paraTableEleAnalyzer()";
    }
    VarSym varSym = *(new VarSym(LEX_NAME, LEX_LONA, symbolTable.convertTypeCode(tmp_type), 0, 0, 0));
    funcSym.parameters.push_back(varSym);
    if (declaration && !errorHandle.checkDupIdenDefine(varSym.lowerName, lexer.lineNum)) {
        symbolTable.insertSymbolToLocal(varSym);
    }
    PRINT_GET
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
    PRINT_MES(("<复合语句>"))
}

/*＜语句列＞   ::= ｛＜语句＞｝*/
void GrammarAnalyzer::statementColumnAnalyzer() {
    if (TOKEN_TYPE != RBRACE) {
        while (TOKEN_TYPE == LBRACE || TOKEN_TYPE == WHILETK || TOKEN_TYPE == FORTK || TOKEN_TYPE == IFTK
               || TOKEN_TYPE == SWITCHTK || TOKEN_TYPE == IDENFR || TOKEN_TYPE == SCANFTK || TOKEN_TYPE == PRINTFTK
               || TOKEN_TYPE == RETURNTK || TOKEN_TYPE == SEMICN) {
            statementAnalyzer();
        }
    }
    // else 语句列为空
    PRINT_MES(("<语句列>"))
}

/*＜语句＞    ::= ＜循环语句＞｜＜条件语句＞| ＜有返回值函数调用语句＞;
 * |＜无返回值函数调用语句＞;｜＜赋值语句＞;｜＜读语句＞;
 * ｜＜写语句＞;｜＜情况语句＞｜＜空＞;|＜返回语句＞; | '{'＜语句列＞'}'*/
void GrammarAnalyzer::statementAnalyzer() {
    // '{'＜语句列＞'}'
    if (TOKEN_TYPE == LBRACE) {
        PRINT_GET
        statementColumnAnalyzer();
        CHECK_GET(RBRACE, "statementAnalyzer()");
    }
        // ＜循环语句＞
    else if (TOKEN_TYPE == WHILETK || TOKEN_TYPE == FORTK) {
        loopStatementAnalyzer();
    }
        // ＜条件语句＞|
    else if (TOKEN_TYPE == IFTK) {
        conditionalStatementAnalyzer();
    }
        // ＜情况语句＞
    else if (TOKEN_TYPE == SWITCHTK) {
        switchStatementAnalyzer();
    } else {
        // ＜有返回值函数调用语句＞;
        // ＜无返回值函数调用语句＞;
        // ＜赋值语句＞;
        if (TOKEN_TYPE == IDENFR) {
            string tmp_lower_name = LEX_LONA;
            PRINT_GET
            if (TOKEN_TYPE == LPARENT) {
                // 函数调用语句
                functionCallAnalyzer(tmp_lower_name);
            } else {
                // 赋值语句
                assignStatementAnalyzer(tmp_lower_name);
            }
        }
            // ＜读语句＞;
        else if (TOKEN_TYPE == SCANFTK) {
            readStatementAnalyzer();
        }
            // ＜写语句＞;
        else if (TOKEN_TYPE == PRINTFTK) {
            writeStatementAnalyzer();
        }
            // ＜返回语句＞;
        else if (TOKEN_TYPE == RETURNTK) {
            returnStatementAnalyzer();
        }
        //;
        CHECK_SEMICN
    }
    PRINT_MES(("<语句>"))
}

/*＜循环语句＞   ::=  while '('＜条件＞')'＜语句＞|
 * for'('＜标识符＞＝＜表达式＞;＜条件＞;＜标识符＞＝＜标识符＞(+|-)＜步长＞')'＜语句＞ */
void GrammarAnalyzer::loopStatementAnalyzer() {
    if (TOKEN_TYPE == WHILETK) {
        PRINT_GET
        CHECK_GET(LPARENT, "whileStatementAnalyzer()");
        conditionAnalyzer();
        // CHECK_//GET(RPARENT, "whileStatementAnalyzer()");
        // while
        CHECK_RPARENT
        statementAnalyzer();
    } else if (TOKEN_TYPE == FORTK) {
        //for
        PRINT_GET
        //'('
        CHECK_GET(LPARENT, "forStatementAnalyzer()");
        //＜标识符＞
        // 未定义的名字
        errorHandle.checkUndefIdenRefer(LEX_LONA, LEX_LINE);
        // 不能改变常量的值
        errorHandle.checkChangeConstIden(LEX_LONA, LEX_LINE);
        CHECK_GET(IDENFR, "forStatementAnalyzer()");
        //＝
        CHECK_GET(ASSIGN, "forStatementAnalyzer()");
        //for'('＜标识符＞＝＜表达式＞
        expressionAnalyzer();
        //;
        //CHECK_//GET(SEMICN, "forStatementAnalyzer()");
        CHECK_SEMICN
        conditionAnalyzer();
        //;
        //CHECK_//GET(SEMICN, "forStatementAnalyzer()");
        CHECK_SEMICN
        //＜标识符＞
        // 未定义的名字
        errorHandle.checkUndefIdenRefer(LEX_LONA, LEX_LINE);
        // 不能改变常量的值
        errorHandle.checkChangeConstIden(LEX_LONA, LEX_LINE);
        CHECK_GET(IDENFR, "forStatementAnalyzer()");
        //＝
        CHECK_GET(ASSIGN, "forStatementAnalyzer()");
        //＜标识符＞
        // 未定义的名字
        errorHandle.checkUndefIdenRefer(LEX_LONA, LEX_LINE);
        CHECK_GET(IDENFR, "forStatementAnalyzer()");
        //(+|-)
        if (TYPE_ADDE) {
            PRINT_GET
        } else {
            cerr << "for";
        }
        //＜步长＞
        unsignedIntAnalyzer();
        PRINT_MES(("<步长>"))
        // ')'
        // CHECK_//GET(RPARENT, "forStatementAnalyzer()");
        // FOR
        CHECK_RPARENT
        // ＜语句＞
        statementAnalyzer();
    }
    PRINT_MES(("<循环语句>"))
}

/*＜条件＞::=  ＜表达式＞＜关系运算符＞＜表达式＞*/
void GrammarAnalyzer::conditionAnalyzer() {
    // 条件判断的左右表达式只能为整型
    if (expressionAnalyzer() != INT) {
        PRINT_G_ERR('f');
    }
    /*＜关系运算符＞  ::=  <｜<=｜>｜>=｜!=｜==*/
    if (TOKEN_TYPE == LSS || TOKEN_TYPE == LEQ || TOKEN_TYPE == GRE ||
        TOKEN_TYPE == GEQ || TOKEN_TYPE == EQL || TOKEN_TYPE == NEQ) {
        PRINT_GET
    } else {
        cerr << "relationalOperatorAnalyzer()";
    }
    if (expressionAnalyzer() != INT) {
        PRINT_G_ERR('f');
    }
    PRINT_MES(("<条件>"))
}

/*＜表达式＞    ::= ［＋｜－］＜项＞{＜加法运算符＞＜项＞}*/
SymbolType GrammarAnalyzer::expressionAnalyzer() {
    SymbolType returnValue = VOID;
    bool calculate = false;
    if (TYPE_ADDE) {
        PRINT_GET
        calculate = true;
    }
    returnValue = itemAnalyzer();
    while (TYPE_ADDE) {
        PRINT_GET
        itemAnalyzer();
        calculate = true;
    }
    PRINT_MES(("<表达式>"))
    return calculate ? INT : returnValue;
}

/*＜项＞     ::= ＜因子＞{＜乘法运算符＞＜因子＞}*/
SymbolType GrammarAnalyzer::itemAnalyzer() {
    SymbolType returnValue = factorAnalyzer();
    while (TOKEN_TYPE == MULT || TOKEN_TYPE == DIV) {
        PRINT_GET
        factorAnalyzer();
        returnValue = INT;
    }
    PRINT_MES(("<项>"))
    return returnValue;
}

/* ＜因子＞    ::=
 * ＜标识符＞｜
 * ＜标识符＞'['＜表达式＞']'|
 * ＜标识符＞'['＜表达式＞']''['＜表达式＞']'|
 * '('＜表达式＞')'｜
 * ＜整数＞|
 * ＜字符＞｜
 * ＜有返回值函数调用语句＞*/
SymbolType GrammarAnalyzer::factorAnalyzer() {
    SymbolType returnValue = VOID;
    if (TOKEN_TYPE == LPARENT) {
        // '('＜表达式＞')'
        PRINT_GET
        expressionAnalyzer();
        CHECK_RPARENT
        // 字符型一旦参与运算则转换成整型，包括小括号括起来的字符型，也算参与了运算，例如(‘c’)的结果是整型
        returnValue = INT;
    } else if (TOKEN_TYPE == IDENFR) {
        string lower_name = LEX_LONA;
        PRINT_GET
        if (TOKEN_TYPE == LBRACK) {
            // ＜标识符＞'['＜表达式＞']''['＜表达式＞']'
            errorHandle.checkUndefIdenRefer(lower_name, LEX_LINE);
            returnValue = symbolTable.getIdenType(lower_name);
            arrayExpAssignAnalyzer();
        } else if (TOKEN_TYPE == LPARENT) {
            // ＜有返回值函数调用语句＞
            returnValue = functionCallAnalyzer(lower_name);
        } else {
            errorHandle.checkUndefIdenRefer(lower_name, LEX_LINE);
            returnValue = symbolTable.getIdenType(lower_name);
        }
    } else if (TOKEN_TYPE == CHARCON) {
        PRINT_GET
        returnValue = CHAR;
    } else {
        bool isInteger = false;
        integerAnalyzer(isInteger);
        if (isInteger) {
            returnValue = INT;
        }
        // check
    }
    PRINT_MES(("<因子>"))
    return returnValue;
}

/*＜条件语句＞  ::= if '('＜条件＞')'＜语句＞［else＜语句＞］*/
void GrammarAnalyzer::conditionalStatementAnalyzer() {
    CHECK_GET(IFTK, "if");
    CHECK_GET(LPARENT, "(");
    conditionAnalyzer();
    //CHECK_//GET(RPARENT, ")");
    // if
    CHECK_RPARENT
    statementAnalyzer();
    if (TOKEN_TYPE == ELSETK) {
        PRINT_GET
        statementAnalyzer();
    }
    PRINT_MES(("<条件语句>"))
}

void GrammarAnalyzer::arrayExpAssignAnalyzer() {
    // 数组元素的下标只能是整型表达式
    // 数组元素的下标不能是字符型
    CHECK_GET(LBRACK, "ASSIGN");
    errorHandle.checkArrayIndexNotInt(expressionAnalyzer(), LEX_LINE);
    CHECK_RBRACK
    if (TOKEN_TYPE == LBRACK) {
        PRINT_GET
        errorHandle.checkArrayIndexNotInt(expressionAnalyzer(), LEX_LINE);
        CHECK_RBRACK
    }
}

/*＜赋值语句＞   ::=  ＜标识符＞＝＜表达式＞|
 * ＜标识符＞'['＜表达式＞']'=＜表达式＞|
 * ＜标识符＞'['＜表达式＞']''['＜表达式＞']' =＜表达式＞*/
void GrammarAnalyzer::assignStatementAnalyzer(string &lower_name) {
    //标识符已读
    // 未定义的名字
    errorHandle.checkUndefIdenRefer(lower_name, LEX_LINE);
    // 不能改变常量的值
    errorHandle.checkChangeConstIden(lower_name, LEX_LINE);
    if (TOKEN_TYPE == LBRACK) {
        arrayExpAssignAnalyzer();
    }
    CHECK_GET(ASSIGN, "ASSIGN =");
    expressionAnalyzer();
    PRINT_MES(("<赋值语句>"))
}

/*＜返回语句＞   ::=  return['('＜表达式＞')']*/
void GrammarAnalyzer::returnStatementAnalyzer() {
    CHECK_GET(RETURNTK, "return keyword");
    if (TOKEN_TYPE == LPARENT) {
        // return (
        PRINT_GET
        if (grammar_func_with_return != VOID && TOKEN_TYPE == RPARENT) {
            // 有返回值的函数有形如return();的语句
            errorHandle.printErrorOfReturnMatch(LEX_LINE);
        } else if (grammar_func_with_return == VOID) {
            // 无返回值的函数出现了形如return(表达式);或return();的语句
            errorHandle.printErrorOfNoReturnMatch(LEX_LINE);
        }
        // 有返回值的函数return语句中表达式类型与返回值类型不一致
        SymbolType expType = expressionAnalyzer();
        if (grammar_func_with_return != VOID && grammar_func_with_return != expType) {
            errorHandle.printErrorOfReturnMatch(LEX_LINE);
        }
        // RETURN
        CHECK_RPARENT
    } else if (grammar_func_with_return != VOID) {
        errorHandle.printErrorOfReturnMatch(LEX_LINE);
    }
    grammar_state_with_return = true;
    PRINT_MES(("<返回语句>"))
}

/*＜读语句＞ ::=  scanf '('＜标识符＞')'*/
void GrammarAnalyzer::readStatementAnalyzer() {
    CHECK_GET(SCANFTK, "SC");
    CHECK_GET(LPARENT, "SC(");
    // 未定义的名字
    errorHandle.checkUndefIdenRefer(LEX_LONA, LEX_LINE);
    // 不能改变常量的值
    errorHandle.checkChangeConstIden(LEX_LONA, LEX_LINE);
    CHECK_GET(IDENFR, "identity");
    //CHECK_//GET(RPARENT, "SC)");
    CHECK_RPARENT
    PRINT_MES(("<读语句>"))
}

/*＜写语句＞    ::=
 * printf '(' ＜字符串＞,＜表达式＞ ')'|
 * printf '('＜字符串＞ ')'|
 * printf '('＜表达式＞')' */
void GrammarAnalyzer::writeStatementAnalyzer() {
    CHECK_GET(PRINTFTK, "PR");
    CHECK_GET(LPARENT, "(");
    // ＜字符串＞
    if (TOKEN_TYPE == STRCON) {
        stringAnalyzer();
        if (TOKEN_TYPE == COMMA) {
            PRINT_GET
            expressionAnalyzer();
        }
    } else {
        expressionAnalyzer();
    }
    //CHECK_//GET(RPARENT, ")");
    CHECK_RPARENT
    PRINT_MES(("<写语句>"))
}

/*{＜有返回值函数定义＞|＜无返回值函数定义＞}＜主函数＞*/
void GrammarAnalyzer::functionDeclarationAnalyzer() {
    do {
        string tmp_name, tmp_lowerName;
        TypeCode tmp_typeCode = WRONG;
        if (TOKEN_TYPE == VOIDTK) {
            // ＜无返回值函数定义＞＜主函数＞
            PRINT_GET
            if (TOKEN_TYPE == MAINTK) {
                // ＜主函数＞
                PRINT_GET
                CHECK_GET(LPARENT, "( main");
                //CHECK_//GET(RPARENT, ") main");
                // 无参数函数定义
                CHECK_RPARENT
                CHECK_GET(LBRACE, "{ main");
                compoundStatementAnalyzer();
                CHECK_GET(RBRACE, "} main");
                symbolTable.endLocalIdenSymbol();
                PRINT_MES(("<主函数>"))
                return;
            } else {
                // ＜无返回值函数定义＞  ::= void＜标识符＞
                FuncSym funcSym = *(new FuncSym(lexer.strToken, lexer.lexToken.content_p, VOID));
                PRINT_GET
                // '('＜参数表＞')''{'＜复合语句＞'}'
                CHECK_GET(LPARENT, "( main");
                parameterTableAnalyzer(funcSym, true);
                //CHECK_//GET(RPARENT, ") main");
                if (!errorHandle.checkDupFuncDefine(funcSym.lowerName, LEX_LINE)) {
                    symbolTable.insertFuncToGlobal(funcSym);
                }
                // 无参数函数定义
                CHECK_RPARENT
                CHECK_GET(LBRACE, "{ main");
                compoundStatementAnalyzer();
                CHECK_GET(RBRACE, "} main");
                symbolTable.endLocalIdenSymbol();
                PRINT_MES(("<无返回值函数定义>"))
            }
        } else if (TYPE_IDEN) {
            // ＜有返回值函数定义＞  ::=  ＜声明头部＞'('＜参数表＞')' '{'＜复合语句＞'}'
            tmp_typeCode = lexer.typeCode;
            PRINT_GET
            tmp_name = lexer.strToken;
            tmp_lowerName = lexer.lexToken.content_p;
            PRINT_GET
            speReValFuncDefAnalyzer(tmp_name, tmp_lowerName, tmp_typeCode);
        }
    } while (true);
}

/*
 * 函数调用语句
 * type = true: must be reFuncCall
 * ＜有返回值函数调用语句＞ ::= ＜标识符＞'('＜值参数表＞')'
 * ＜无返回值函数调用语句＞ ::= ＜标识符＞'('＜值参数表＞')'
 */
SymbolType GrammarAnalyzer::functionCallAnalyzer(string &lower_name) {
    // 返回函数类型
    SymbolType funcType = symbolTable.getFuncType(lower_name);
    if (!errorHandle.checkUndefFuncCall(lower_name, LEX_LINE)) {
        CHECK_GET(LPARENT, "func call (");
        vector<VarSym> vectorVar = symbolTable.getFuncParams(lower_name);
        valParaTableAnalyzer(vectorVar);
        //CHECK_//GET(RPARENT, "func call )");
        // 有/无参数函数调用
        CHECK_RPARENT
        if (funcType != VOID) {
            PRINT_MES(("<有返回值函数调用语句>"))
        } else {
            PRINT_MES(("<无返回值函数调用语句>"))
        }
    } else {
        while (TOKEN_TYPE != RPARENT) {
            PRINT_GET
        }
        PRINT_GET
    }
    return funcType;
}

/*＜值参数表＞   ::= ＜表达式＞{,＜表达式＞}｜＜空＞*/
void GrammarAnalyzer::valParaTableAnalyzer(vector<VarSym> &vectorVar) {
    vector<SymbolType> referParamsList;
    if (TOKEN_TYPE == RPARENT) {
        // ＜空＞
    } else {
        SymbolType st = expressionAnalyzer();
        if (st != VOID) {
            referParamsList.push_back(st);
        }
        while (TOKEN_TYPE == COMMA) {
            PRINT_GET
            st = expressionAnalyzer();
            if (st != VOID) {
                referParamsList.push_back(st);
            }
        }
    }
    if (vectorVar.size() != referParamsList.size()) {
        // 函数参数个数不匹配
        // 函数调用时实参个数大于或小于形参个数
        PRINT_G_ERR('d');
    } else {
        for (int i = 0; i < referParamsList.size(); i++) {
            if (vectorVar.at(i).symbolType != referParamsList.at(i)) {
                // 函数参数类型不匹配
                PRINT_G_ERR('e');
                break;
            }
        }
    }
    PRINT_MES(("<值参数表>"))
}

/*＜字符串＞   ::=  "｛十进制编码为32,33,35-126的ASCII字符｝"*/
void GrammarAnalyzer::stringAnalyzer() {
    CHECK_GET(STRCON, "str con");
    PRINT_MES(("<字符串>"))
}

/*＜情况语句＞  ::=  switch ‘(’＜表达式＞‘)’ ‘{’＜情况表＞＜缺省＞‘}’*/
void GrammarAnalyzer::switchStatementAnalyzer() {
    CHECK_GET(SWITCHTK, "switch");
    CHECK_GET(LPARENT, "func call (");
    SymbolType switchType = expressionAnalyzer();
    //CHECK_//GET(RPARENT, "func call )");
    CHECK_RPARENT
    CHECK_GET(LBRACE, "{ main");
    situationSwitchAnalyzer(switchType);
    if (!defaultSwitchAnalyzer()) {
        // switch语句中，缺少<缺省>语句。
        errorHandle.printErrorLine('p', LEX_LINE);
    }
    CHECK_GET(RBRACE, "} main");
    PRINT_MES(("<情况语句>"))
}

/*＜情况表＞   ::=  ＜情况子语句＞{＜情况子语句＞} */
void GrammarAnalyzer::situationSwitchAnalyzer(SymbolType switchType) {
    do {
        // ＜情况子语句＞  ::=  case＜常量＞：＜语句＞
        CHECK_GET(CASETK, "CASE");
        if (constantAnalyzer() != switchType) {
            errorHandle.printErrorOfUnMatchConst(LEX_LINE);
        }
        CHECK_GET(COLON, "CASE");
        statementAnalyzer();
        PRINT_MES(("<情况子语句>"))
    } while (TOKEN_TYPE == CASETK);
    PRINT_MES(("<情况表>"))
}

/*＜缺省＞   ::=  default :＜语句＞*/
bool GrammarAnalyzer::defaultSwitchAnalyzer() {
    if (TOKEN_TYPE == DEFAULTTK) {
        // CHECK_GET(DEFAULTTK, "default");
        PRINT_GET;
        CHECK_GET(COLON, "default : ");
        statementAnalyzer();
        PRINT_MES(("<缺省>"))
        return true;
    }
    return false;
}

void GrammarAnalyzer::readToRightBrack() {
    // 数组初始化个数不匹配
    PRINT_G_ERR('n')
    while (TOKEN_TYPE != RBRACE) {
        PRINT_GET
    }
    PRINT_GET
}





