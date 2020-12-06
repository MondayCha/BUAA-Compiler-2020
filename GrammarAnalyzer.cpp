//
// Created by Dlee on 2020/10/5.
//

#include "include/GrammarAnalyzer.h"
#include <algorithm>
#include <utility>

SymbolType grammar_func_with_return;
bool grammar_state_with_return;
int grammar_var_offset;
bool grammar_meet_main;

GrammarAnalyzer::GrammarAnalyzer(Lexer &pronLexer, SymbolTable &pronSymbol, ErrorHandle &pronError,
                                 IRCodeManager &pronCode, std::ofstream &pronFile)
        : lexer(pronLexer), symbolTable(pronSymbol), errorHandle(pronError), irCode(pronCode), outFile(pronFile) {
    grammar_func_with_return = VOID;
    grammar_state_with_return = false;
    grammar_var_offset = 0;
    grammar_meet_main = false;
}

GrammarAnalyzer &GrammarAnalyzer::getInstance(Lexer &pronLexer, SymbolTable &pronSymbol, ErrorHandle &pronError,
                                              IRCodeManager &pronCode, std::ofstream &pronOutFile) {
    static GrammarAnalyzer instance(pronLexer, pronSymbol, pronError, pronCode, pronOutFile);
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
        endConstOrVarDeclaration();
    }
    // {＜有返回值函数定义＞|＜无返回值函数定义＞}＜主函数＞
    functionDeclarationAnalyzer();
    ADD_MIDCODE(OpExit, "", "", "")
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
                symbolTable.insertSymbolToLocal(new ConSym(name, lowerName, INT, content));
                ADD_MIDCODE(OpConst, INT, lowerName, content)
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
            if (!errorHandle.checkDupIdenDefine(lowerName, lexer.lineNum)) {
                symbolTable.insertSymbolToLocal(new ConSym(name, lowerName, CHAR, content));
                ADD_MIDCODE(OpConst, CHAR, lowerName, content)
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
        tmp_name = LEX_NAME;
        tmp_lowerName = LEX_LONA;
        lexer.getNextToken();
        // ＜有返回值函数定义＞
        if (TOKEN_TYPE == LPARENT) {
            endConstOrVarDeclaration();
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
    endConstOrVarDeclaration();
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
    int level = 0, length1 = 0, length2 = 0, needSpace = 0;
    SymbolType symType = symbolTable.convertTypeCode(typeCode);
    if (TOKEN_TYPE == LBRACK) {
        arrayVarAnalyzer(level, length1, length2);
    }
    needSpace = getNeedSpace(level, length1, length2);
    VarSym *var_p = new VarSym(name, lowerName, symType, level, length1, length2, grammar_var_offset);
    bool var_valid = !errorHandle.checkDupIdenDefine(lowerName, LEX_LINE);
    if (var_valid) {
        symbolTable.insertSymbolToLocal(var_p);
        grammar_var_offset += needSpace;
        ADD_MIDCODE(OpVar, symType, lowerName, needSpace)
    }
    if (TOKEN_TYPE == ASSIGN) {
        // 变量定义及初始化
        PRINT_GET
        // 初始化
        varInitAnalyzer(lowerName, level, length1, length2, 0, (typeCode == INTTK));
        //        string exp_string;
        //        int exp_int;
        //        SymbolType tmpType = expressionAnalyzer(exp_string, exp_int);
        //        if (tmpType == CHAR || tmpType == INT) {
        //            ADD_MIDCODE(OpASSIGN, lowerName, exp_int, "")
        //        } else {
        //            ADD_MIDCODE(OpASSIGN, lowerName, exp_string, "")
        //        }
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
            needSpace = getNeedSpace(level, length1, length2);
            if (!errorHandle.checkDupIdenDefine(subLowerName, LEX_LINE)) {
                symbolTable.insertSymbolToLocal(
                        new VarSym(subName, subLowerName, symType, level, length1, length2, grammar_var_offset));
                grammar_var_offset += needSpace;
                ADD_MIDCODE(OpVar, symType, subLowerName, needSpace)
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
    SymbolType symbolType = symbolTable.convertTypeCode(tk);
    auto *funcSym = new FuncSym(name, lowerName, symbolType);
    ADD_MIDCODE(OpFunc, symbolType, lowerName, "")
    parameterTableAnalyzer(*funcSym, true);
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
    compoundStatementAnalyzer();
    //'}'
    if (!grammar_state_with_return) {
        errorHandle.printErrorOfReturnMatch(LEX_LINE);
    }
    CHECK_GET(RBRACE, "speReValFuncDefAnalyzer }");
    PRINT_MES(("<有返回值函数定义>"))
//    cerr << "before" << endl;
//    symbolTable.printMap(&(funcSym->funcLocalTable), "funcLocalTable");
//    symbolTable.printMap(&(symbolTable.localIdenTable), "localIdenTable");
//    symbolTable.printMap(&(symbolTable.globalIdenTable), "globalIdenTable");
    funcSym->setFuncLocalTable(symbolTable.localIdenTable);
    symbolTable.localIdenTable.clear();
//    cerr << "after" << endl;
//    symbolTable.printMap(&(funcSym->funcLocalTable), "funcLocalTable");
//    symbolTable.printMap(&(symbolTable.localIdenTable), "localIdenTable");
//    symbolTable.printMap(&(symbolTable.globalIdenTable), "globalIdenTable");
    funcSym->funcSpace = grammar_var_offset;
    grammar_var_offset = 0;
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
void GrammarAnalyzer::varInitAnalyzer(string &lowerName, int level, int length1, int length2,
                                      int offset, bool isInteger) {
    if (level == 0) {
        // 不会多一维的
        irCode.addThreeAddCode(
                new ThreeAddCode(OpASSIGN, lowerName, constInVarInitAnalyzer_return_value(isInteger), ""));
    } else if (level == 1) {
        // 一维数组
        // {
        PRINT_GET
        int i = 0;
        for (; i < length1 - 1; i++) {
            irCode.addThreeAddCode(
                    new ThreeAddCode(OpAssArray, lowerName, offset + i,
                                     constInVarInitAnalyzer_return_value(isInteger)));
            if (TOKEN_TYPE != COMMA) { ; //error
                // 遇到了提前结束的情况
                // 如：int a[4] = {0,1}
                readToRightBrack_and_log_error();
                return;
            }
            PRINT_GET
        }
        irCode.addThreeAddCode(
                new ThreeAddCode(OpAssArray, lowerName, offset + i,
                                 constInVarInitAnalyzer_return_value(isInteger)));
        if (TOKEN_TYPE != RBRACE) { ; //error
            readToRightBrack_and_log_error();
            return;
        }
        PRINT_GET
    } else {
        // 外层大括号
        PRINT_GET
        // 读取{}，
        for (int i = 0; i < length1 - 1; i++) {
            varInitAnalyzer(lowerName, 1, length2, length2, offset, isInteger);
            if (TOKEN_TYPE != COMMA) { ; //error
                readToRightBrack_and_log_error();
                return;
            }
            offset += length2;
            PRINT_GET
        }
        varInitAnalyzer(lowerName, 1, length2, length2, offset, isInteger);
        if (TOKEN_TYPE == RBRACE) {
            PRINT_GET
            return;
        }
        if (TOKEN_TYPE == COMMA) {
            PRINT_GET
        }
        readToRightBrack_and_log_error();
    }
}

/*＜常量＞   ::=  ＜整数＞|＜字符＞*/
int GrammarAnalyzer::constInVarInitAnalyzer_return_value(bool isInteger) {
    if (isInteger) {
        if (TOKEN_TYPE == CHARCON) {
            errorHandle.printErrorOfUnMatchConst(LEX_LINE);
            int returnValue = lexer.lexToken.contentNum;
            PRINT_GET
            return returnValue;
        } else {
            return integerAnalyzer();
        }
    } else {
        if (TOKEN_TYPE != CHARCON) {
            //is int
            errorHandle.printErrorOfUnMatchConst(LEX_LINE);
            return integerAnalyzer();
        } else {
            int returnValue = lexer.lexToken.contentNum;
            PRINT_GET
            return returnValue;
        }
    }
    PRINT_MES(("<常量>"))
    return 0;
}

SymbolType GrammarAnalyzer::constantAnalyzer(int &switchConstant) {
    SymbolType symbolType = VOID;
    if (TOKEN_TYPE == CHARCON) {
        switchConstant = lexer.lexToken.contentNum;
        PRINT_GET
        symbolType = CHAR;
    } else {
        switchConstant = integerAnalyzer();
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
}

/*＜类型标识符＞＜标识符＞*/
void GrammarAnalyzer::paraTableEleAnalyzer(FuncSym &funcSym, bool declaration) {
    if (!TYPE_IDEN) {
        cerr << "paraTableEleAnalyzer()";
    }
    SymbolType symType = symbolTable.convertTypeCode(lexer.typeCode);
    PRINT_GET
    if (TOKEN_TYPE != IDENFR) {
        cerr << "paraTableEleAnalyzer()";
    }
    VarSym *varSym = new VarSym(LEX_NAME, LEX_LONA, symType, 0, 0, 0,
                                grammar_var_offset);
    grammar_var_offset++;
    funcSym.parameters.push_back(*varSym);
    if (declaration && !errorHandle.checkDupIdenDefine(varSym->lowerName, lexer.lineNum)) {
        symbolTable.insertSymbolToLocal(varSym);
    }
    ADD_MIDCODE(OpParam, symType, LEX_LONA, "")
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
        while (TOKEN_TYPE == LBRACE || TOKEN_TYPE == WHILETK || TOKEN_TYPE == FORTK || TOKEN_TYPE == IFTK ||
               TOKEN_TYPE == SWITCHTK || TOKEN_TYPE == IDENFR || TOKEN_TYPE == SCANFTK || TOKEN_TYPE == PRINTFTK ||
               TOKEN_TYPE == RETURNTK || TOKEN_TYPE == SEMICN) {
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
            string tmp_name = LEX_NAME;
            PRINT_GET
            if (TOKEN_TYPE == LPARENT) {
                // 函数调用语句
                functionCallAnalyzer(tmp_lower_name);
            } else {
                // 赋值语句
                assignStatementAnalyzer(tmp_name, tmp_lower_name);
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
        string condition_var, label1, label2;
        label1 = genLabel();
        ADD_MIDCODE(OpLabel, label1, "", "")
        conditionAnalyzer(condition_var);
        CHECK_RPARENT
        label2 = genLabel();
        ADD_MIDCODE(OpBEZ, label2, condition_var, "")
        statementAnalyzer();
        ADD_MIDCODE(OpJmp, label1, "", "")
        ADD_MIDCODE(OpLabel, label2, "", "")
    } else if (TOKEN_TYPE == FORTK) {
        string lowerName1, lowerName2, lowerName3;
        string exp_str, for_var, label1, label2;
        SymbolType exp_type;
        int exp_int;
        //for
        PRINT_GET
        //'('
        CHECK_GET(LPARENT, "forStatementAnalyzer (");
        lowerName1 = LEX_LONA; //＜标识符＞
        // 未定义的名字
        errorHandle.checkUndefIdenRefer(LEX_LONA, LEX_LINE);
        // 不能改变常量的值
        errorHandle.checkChangeConstIden(LEX_LONA, LEX_LINE);
        CHECK_GET(IDENFR, "forStatementAnalyzer ");
        CHECK_GET(ASSIGN, "forStatementAnalyzer ="); //＝
        //for'('＜标识符＞＝＜表达式＞
        exp_type = expressionAnalyzer(exp_str, exp_int);
        if (exp_type == INT) {
            ADD_MIDCODE(OpASSIGN, lowerName1, exp_int, "")
        } else {
            ADD_MIDCODE(OpASSIGN, lowerName1, exp_str, "")
        }
        label1 = genLabel();
        ADD_MIDCODE(OpLabel, label1, "", "")
        CHECK_SEMICN //;
        conditionAnalyzer(for_var);
        label2 = genLabel();
        ADD_MIDCODE(OpBEZ, label2, for_var, "")
        CHECK_SEMICN //;
        //＜标识符＞
        lowerName2 = LEX_LONA;
        // 未定义的名字
        errorHandle.checkUndefIdenRefer(LEX_LONA, LEX_LINE);
        // 不能改变常量的值
        errorHandle.checkChangeConstIden(LEX_LONA, LEX_LINE);
        CHECK_GET(IDENFR, "forStatementAnalyzer()");
        //＝
        CHECK_GET(ASSIGN, "forStatementAnalyzer()");
        //＜标识符＞
        lowerName3 = LEX_LONA;
        // 未定义的名字
        errorHandle.checkUndefIdenRefer(LEX_LONA, LEX_LINE);
        CHECK_GET(IDENFR, "forStatementAnalyzer()");
        int stepSize;
        bool isPlus = true;
        //(+|-)
        if (TYPE_ADDE) {
            if (TOKEN_TYPE == MINU) {
                isPlus = false;
            }
            PRINT_GET
        }
        //＜步长＞
        stepSize = unsignedIntAnalyzer();
        PRINT_MES(("<步长>"))
        CHECK_RPARENT
        // ＜语句＞
        statementAnalyzer();
        ADD_MIDCODE(isPlus ? OpPLUS : OpMINU, lowerName2, lowerName3, stepSize)
        ADD_MIDCODE(OpJmp, label1, "", "")
        ADD_MIDCODE(OpLabel, label2, "", "")
    }
    PRINT_MES(("<循环语句>"))
}

/*＜条件＞::=  ＜表达式＞＜关系运算符＞＜表达式＞*/
void GrammarAnalyzer::conditionAnalyzer(string &condition_var) {
    // 条件判断的左右表达式只能为整型
    string exp1_str, exp2_str;
    int exp1_int, exp2_int;
    SymbolType exp1_type, exp2_type;
    exp1_type = expressionAnalyzer(exp1_str, exp1_int);
    if (exp1_type != INT && exp1_type != INTVAR) {
        PRINT_G_ERR('f');
    }
    /*＜关系运算符＞  ::=  <｜<=｜>｜>=｜!=｜==*/
    OperatorType opType;
    if (TOKEN_TYPE == LSS) {
        opType = OpLSS;
    } else if (TOKEN_TYPE == LEQ) {
        opType = OpLEQ;
    } else if (TOKEN_TYPE == GRE) {
        opType = OpGRE;
    } else if (TOKEN_TYPE == GEQ) {
        opType = OpGEQ;
    } else if (TOKEN_TYPE == EQL) {
        opType = OpEQL;
    } else if (TOKEN_TYPE == NEQ) {
        opType = OpNEQ;
    } else {
        cerr << "relationalOperatorAnalyzer()";
    }
    PRINT_GET
    exp2_type = expressionAnalyzer(exp2_str, exp2_int);
    if (exp2_type != INT && exp2_type != INTVAR) {
        PRINT_G_ERR('f');
    }
    condition_var = genTmpVar_and_insert(INT);
    if (exp1_type == INT && exp2_type == INT) {
        ADD_MIDCODE(opType, condition_var, exp1_int, exp2_int)
    } else if (exp1_type != INT && exp2_type == INT) {
        ADD_MIDCODE(opType, condition_var, exp1_str, exp2_int)
    } else if (exp1_type == INT && exp2_type != INT) {
        ADD_MIDCODE(opType, condition_var, exp1_int, exp2_str)
    } else {
        ADD_MIDCODE(opType, condition_var, exp1_str, exp2_str)
    }
    PRINT_MES(("<条件>"))
}

/*＜表达式＞    ::= ［＋｜－］＜项＞{＜加法运算符＞＜项＞}*/
SymbolType GrammarAnalyzer::expressionAnalyzer() {
    string exp_str;
    return expressionAnalyzer(exp_str);
}

SymbolType GrammarAnalyzer::expressionAnalyzer(string &exp_str) {
    int exp_int;
    SymbolType ans = expressionAnalyzer(exp_str, exp_int);
    if (ans == INTVAR) {
        return INT;
    } else if (ans == CHARVAR) {
        return CHAR;
    } else {
        return ans;
    }
}

// 返回CHAR: 单个char类型常量
// 返回CHARVAR: 单个char类型变量
// 返回INT: 单个int类型常量
// 返回INTVAR: 复合个int类型/变量
// 返回VOID: 空
SymbolType GrammarAnalyzer::expressionAnalyzer(string &exp_str, int &exp_int) {
#ifdef CODE_OPTIMIZE_ON
    SymbolType itemType = VOID;     // 返回的表达式类型值
    bool containAddSub = false;     // 包含计算类型
    bool isMinus = false;           // 第一项之前为-
    bool expHasVar = true;         // op1存储了表达式之前的变量结果
    int expConstValue = 0;          // 表达式的常数部分
    string ans, op1, op2;           // ans:结果 op1:操作数1 op2:操作数2
    if (TYPE_ADDE) {
        containAddSub = true;
        isMinus = (TOKEN_TYPE == MINU);
        PRINT_GET
    }
    // 读取表达式的第一项到op1
    itemType = itemAnalyzer(op1, exp_int);  // expHasVar = true;
    if (itemType == CHAR || itemType == INT) {  // 是常量
        expHasVar = false;
        expConstValue = (containAddSub && isMinus) ? -exp_int : exp_int; // 第一项之前为+或-, 字符类型修改为int类型
    } else if (isMinus) {   // 是变量并且第一项之前为-, 需要更新op1的值
        STORE_EXP(OpMINU, 0, op1)
    }
    while (TYPE_ADDE) {
        containAddSub = true;
        isMinus = (TOKEN_TYPE == MINU);
        PRINT_GET
        itemType = itemAnalyzer(op2, exp_int);
        if (itemType == CHAR || itemType == INT) {  // 是常量
            expConstValue += isMinus ? (-exp_int) : (exp_int);
        } else {                                    // 是变量
            if (expHasVar) {
                STORE_EXP(isMinus ? OpMINU : OpPLUS, op1, op2)
            } else {
                expHasVar = true;
                if (isMinus) {
                    STORE_EXP(OpMINU, 0, op2)
                } else {
                    op1 = move(op2);
                }
            }
        }
    }
    if (expHasVar && expConstValue != 0) {
        STORE_EXP(OpPLUS, op1, expConstValue)
    }
    exp_str = op1;
    PRINT_MES(("<表达式>"))
    if (containAddSub) {
        if (expHasVar) {
            return INTVAR;
        } else {
            exp_int = expConstValue;
            return INT;
        }
    } else {
        return itemType;
    }
#elif
    SymbolType itemType = VOID;
    bool containAddSub = false;
    bool firstAddSub = false;
    bool isMinus = false;
    if (TYPE_ADDE) {
        isMinus = (TOKEN_TYPE == MINU);
        PRINT_GET
        firstAddSub = true;
    }
    string ans, op1, op2;
    itemType = itemAnalyzer(op1, exp_int);
    bool op1isConst = (itemType == CHAR || itemType == INT);
    int op1Int = exp_int;
    if (firstAddSub && (itemType == CHAR || itemType == CHARVAR)) {
        containAddSub = true;
        if (itemType == CHAR) {
            STORE_EXP(isMinus ? OpMINU : OpPLUS, 0, exp_int)
        } else {
            STORE_EXP(isMinus ? OpMINU : OpPLUS, 0, op1)
        }
        op1isConst = false;
    } else if (firstAddSub && isMinus) {
        if (op1isConst) {
            exp_int = -exp_int;
            op1Int = exp_int;
            op1 = to_string(exp_int);
        } else {
            containAddSub = true;
            STORE_EXP(OpMINU, 0, op1)
        }
    }
    while (TYPE_ADDE) {
        isMinus = (TOKEN_TYPE == MINU);
        PRINT_GET
        containAddSub = true;
        SymbolType tmpType = itemAnalyzer(op2, exp_int);
        bool op2isInt = (tmpType == CHAR || tmpType == INT);
        if (op1isConst && op2isInt) {
            STORE_EXP(isMinus ? OpMINU : OpPLUS, op1Int, exp_int)
            op1isConst = false;
        } else if (op1isConst && !op2isInt) {
            STORE_EXP(isMinus ? OpMINU : OpPLUS, op1Int, op2)
            op1isConst = false;
        } else if (!op1isConst && op2isInt) {
            STORE_EXP(isMinus ? OpMINU : OpPLUS, op1, exp_int)
        } else {
            STORE_EXP(isMinus ? OpMINU : OpPLUS, op1, op2)
        }
    }
    exp_str = op1;
    PRINT_MES(("<表达式>"))
    return containAddSub ? INTVAR : itemType;
#endif
}

/*＜项＞     ::= ＜因子＞{＜乘法运算符＞＜因子＞}*/
SymbolType GrammarAnalyzer::itemAnalyzer(string &exp_str, int &exp_int) {
#ifdef CODE_OPTIMIZE_ON
    string ans, op1, op2;
    bool isMult = false;
    // ＜因子＞
    SymbolType returnValue = factorAnalyzer(op1, exp_int);
    bool op1isInt = (returnValue == CHAR || returnValue == INT);
    int op1Int = exp_int;
    // {＜乘法运算符＞＜因子＞}
    while (TOKEN_TYPE == MULT || TOKEN_TYPE == DIV) {
        isMult = (TOKEN_TYPE == MULT);
        PRINT_GET
        returnValue = INTVAR;
        SymbolType tmpType = factorAnalyzer(op2, exp_int);
        bool op2isInt = (tmpType == CHAR || tmpType == INT);
        if (op1isInt && op2isInt) {
            STORE_EXP(isMult ? OpMULT : OpDIV, op1Int, exp_int)
            op1isInt = false;
        } else if (op1isInt && !op2isInt) {
            STORE_EXP(isMult ? OpMULT : OpDIV, op1Int, op2)
            op1isInt = false;
        } else if (!op1isInt && op2isInt) {
            STORE_EXP(isMult ? OpMULT : OpDIV, op1, exp_int)
        } else {
            STORE_EXP(isMult ? OpMULT : OpDIV, op1, op2)
        }
    }
    exp_str = op1;
    PRINT_MES(("<项>"))
    return returnValue;
#elif
    string ans, op1, op2;
    bool isMult = false;
    // ＜因子＞
    SymbolType returnValue = factorAnalyzer(op1, exp_int);
    bool op1isInt = (returnValue == CHAR || returnValue == INT);
    int op1Int = exp_int;
    // {＜乘法运算符＞＜因子＞}
    while (TOKEN_TYPE == MULT || TOKEN_TYPE == DIV) {
        isMult = (TOKEN_TYPE == MULT);
        PRINT_GET
        returnValue = INTVAR;
        SymbolType tmpType = factorAnalyzer(op2, exp_int);
        bool op2isInt = (tmpType == CHAR || tmpType == INT);
        if (op1isInt && op2isInt) {
            STORE_EXP(isMult ? OpMULT : OpDIV, op1Int, exp_int)
            op1isInt = false;
        } else if (op1isInt && !op2isInt) {
            STORE_EXP(isMult ? OpMULT : OpDIV, op1Int, op2)
            op1isInt = false;
        } else if (!op1isInt && op2isInt) {
            STORE_EXP(isMult ? OpMULT : OpDIV, op1, exp_int)
        } else {
            STORE_EXP(isMult ? OpMULT : OpDIV, op1, op2)
        }
    }
    exp_str = op1;
    PRINT_MES(("<项>"))
    return returnValue;
#endif
}

/* ＜因子＞    ::=
 * ＜标识符＞｜
 * ＜标识符＞'['＜表达式＞']'|
 * ＜标识符＞'['＜表达式＞']''['＜表达式＞']'|
 * '('＜表达式＞')'｜
 * ＜整数＞|
 * ＜字符＞｜
 * ＜有返回值函数调用语句＞*/
SymbolType GrammarAnalyzer::factorAnalyzer(string &exp_str, int &exp_int) {
    SymbolType returnValue = VOID;
    if (TOKEN_TYPE == LPARENT) {
        // '('＜表达式＞')'
        PRINT_GET
        SymbolType sy = expressionAnalyzer(exp_str, exp_int);
        CHECK_RPARENT
        // 字符型一旦参与运算则转换成整型，包括小括号括起来的字符型，也算参与了运算，例如(‘c’)的结果是整型
        returnValue = (sy == CHAR) ? INTVAR : sy;
    } else if (TOKEN_TYPE == IDENFR) {
        string lower_name = LEX_LONA;
        PRINT_GET
        if (TOKEN_TYPE == LBRACK) {
            // ＜标识符＞'['＜表达式＞']''['＜表达式＞']'
            errorHandle.checkUndefIdenRefer(lower_name, LEX_LINE);
            returnValue = symbolTable.getIdenType(lower_name);
            exp_str = arrayExpAssignAnalyzer(lower_name);
        } else if (TOKEN_TYPE == LPARENT) {
            // ＜有返回值函数调用语句＞
            returnValue = functionCallAnalyzer(lower_name);
            exp_str = genTmpVar_and_insert(returnValue);
            ADD_MIDCODE(OpRetVar, returnValue, exp_str, "")
        } else {
            errorHandle.checkUndefIdenRefer(lower_name, LEX_LINE);
            returnValue = symbolTable.getIdenType(lower_name);
            exp_str = LEX_LONA;
        }
        if (returnValue == INT) {
            returnValue = INTVAR;
        } else if (returnValue == CHAR) {
            returnValue = CHARVAR;
        }
    } else if (TOKEN_TYPE == CHARCON) {
        exp_int = lexer.lexToken.contentNum;
        exp_str = to_string(exp_int);
        PRINT_GET
        returnValue = CHAR;
    } else {
        bool isInteger = false;
        exp_int = integerAnalyzer(isInteger);
        exp_str = to_string(exp_int);
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
    // if '('＜条件＞')'
    string condition_var;
    CHECK_GET(IFTK, "if");
    CHECK_GET(LPARENT, "(");
    conditionAnalyzer(condition_var);
    CHECK_RPARENT
    string label_if_else, label_if_end;
    label_if_else = genLabel();
    ADD_MIDCODE(OpBEZ, label_if_else, condition_var, "")
    statementAnalyzer();
    if (TOKEN_TYPE == ELSETK) {
        PRINT_GET
        label_if_end = genLabel();
        ADD_MIDCODE(OpJmp, label_if_end, "", "")
        ADD_MIDCODE(OpLabel, label_if_else, "", "")
        statementAnalyzer();
        ADD_MIDCODE(OpLabel, label_if_end, "", "")
    } else {
        ADD_MIDCODE(OpLabel, label_if_else, "", "")
    }
    PRINT_MES(("<条件语句>"))
}

string GrammarAnalyzer::arrayExpAssignAnalyzer(string &lower_name) {
    string x;
    int y;
    return arrayExpAssignAnalyzer(lower_name, true, x, y);
}

string GrammarAnalyzer::arrayExpAssignAnalyzer(string &lower_name, bool genTmp, string &index_s, int &index) {
    // 数组元素的下标只能是整型表达式
    // 数组元素的下标不能是字符型
    CHECK_GET(LBRACK, "ASSIGN");
    string str_len1, str_len2;
    int exp_int1, exp_int2;
    int level = 1;
    SymbolType t1, t2;
    t1 = expressionAnalyzer(str_len1, exp_int1);
    errorHandle.checkArrayIndexNotInt(t1, LEX_LINE);
    CHECK_RBRACK
    if (TOKEN_TYPE == LBRACK) {
        PRINT_GET
        level = 2;
        t2 = expressionAnalyzer(str_len2, exp_int2);
        errorHandle.checkArrayIndexNotInt(t2, LEX_LINE);
        CHECK_RBRACK
    }
    if (level == 1) {
        // 一维数组
        string ans;
        if (t1 == INT) {
            if (genTmp) {
                ans = genTmpVar_and_insert(INT);
                ADD_MIDCODE(OpGetArray, ans, lower_name, exp_int1)
            }
            index = exp_int1;
        } else {
            if (genTmp) {
                ans = genTmpVar_and_insert(INT);
                ADD_MIDCODE(OpGetArray, ans, lower_name, str_len1)
            }
            index = -1;
            index_s = str_len1;
        }
        return ans;
    } else {
        bool isGlobal;
        auto *var_p = (VarSym *) symbolTable.getSymbolPtr(&(symbolTable.localIdenTable), lower_name, isGlobal);
        if (var_p == nullptr) {
            return nullptr;
        }
        int off;
        string ans, tmp1, tmp2;
        if (t1 == INT && t2 == INT) {
            ans = genTmpVar_and_insert(INT);
            off = exp_int1 * var_p->length2 + exp_int2;
            if (genTmp) {
                ADD_MIDCODE(OpGetArray, ans, lower_name, off)
            }
            index = off;
        } else {
            index = -1;
            if (t1 != INT && t2 != INT) {
                tmp1 = genTmpVar_and_insert(INT);
                tmp2 = genTmpVar_and_insert(INT);
                ans = genTmpVar_and_insert(INT);
                ADD_MIDCODE(OpMULT, tmp1, str_len1, var_p->length2)
                ADD_MIDCODE(OpPLUS, tmp2, tmp1, str_len2)
            } else if (t1 != INT && t2 == INT) {
                tmp1 = genTmpVar_and_insert(INT);
                tmp2 = genTmpVar_and_insert(INT);
                ans = genTmpVar_and_insert(INT);
                ADD_MIDCODE(OpMULT, tmp1, str_len1, var_p->length2)
                ADD_MIDCODE(OpPLUS, tmp2, tmp1, exp_int2)
            } else {
                tmp2 = genTmpVar_and_insert(INT);
                ans = genTmpVar_and_insert(INT);
                ADD_MIDCODE(OpPLUS, tmp2, str_len2, (exp_int1 * var_p->length2))
            }
            if (genTmp) {
                ADD_MIDCODE(OpGetArray, ans, lower_name, tmp2)
            }
            index_s = move(tmp2);
        }
        return ans;
    }
}

/*＜赋值语句＞   ::=  ＜标识符＞＝＜表达式＞|
 * ＜标识符＞'['＜表达式＞']'=＜表达式＞|
 * ＜标识符＞'['＜表达式＞']''['＜表达式＞']' =＜表达式＞*/
void GrammarAnalyzer::assignStatementAnalyzer(string &name, string &lower_name) {
    //标识符已读
    // 未定义的名字
    errorHandle.checkUndefIdenRefer(lower_name, LEX_LINE);
    // 不能改变常量的值
    errorHandle.checkChangeConstIden(lower_name, LEX_LINE);
    bool isArray = false;
    string array_index_s;
    int array_index;
    if (TOKEN_TYPE == LBRACK) {
        isArray = true;
        arrayExpAssignAnalyzer(lower_name, false, array_index_s, array_index);
    }
    CHECK_GET(ASSIGN, "ASSIGN =");
    string exp_string;
    int exp_int;
    SymbolType tType = expressionAnalyzer(exp_string, exp_int);
    if (tType == CHAR || tType == INT) {
        if (isArray) {
            if (array_index < 0) {
                ADD_MIDCODE(OpAssArray, lower_name, array_index_s, exp_int)
            } else {
                ADD_MIDCODE(OpAssArray, lower_name, array_index, exp_int)
            }
        } else {
            ADD_MIDCODE(OpASSIGN, lower_name, exp_int, "")
        }
    } else {
        if (isArray) {
            if (array_index < 0) {
                ADD_MIDCODE(OpAssArray, lower_name, array_index_s, exp_string)
            } else {
                ADD_MIDCODE(OpAssArray, lower_name, array_index, exp_string)
            }
        } else {
//#ifdef CODE_OPTIMIZE_ON
//            // 消去最后一个临时变量
//            irCode.updateLastCode(lower_name);
//#elif
            ADD_MIDCODE(OpASSIGN, lower_name, exp_string, "")
//#endif
        }
    }
    PRINT_MES(("<赋值语句>"))
}

/*＜返回语句＞   ::=  return['('＜表达式＞')']*/
void GrammarAnalyzer::returnStatementAnalyzer() {
    bool addReturnMidcode = false;
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
        string exp_str;
        int exp_int;
        SymbolType expType = expressionAnalyzer(exp_str, exp_int);
        SymbolType exp_type = expType;
        if (expType == INTVAR) {
            expType = INT;
        } else if (expType == CHARVAR) {
            expType = CHAR;
        }
        if (grammar_func_with_return != VOID && grammar_func_with_return != expType) {
            errorHandle.printErrorOfReturnMatch(LEX_LINE);
        }
        if (exp_type == INT && exp_type == CHAR) {
            ADD_MIDCODE(OpReturn, expType, exp_int, "")
        } else {
            ADD_MIDCODE(OpReturn, expType, exp_str, "")
        }
        addReturnMidcode = true;
        // RETURN
        CHECK_RPARENT
    } else if (grammar_func_with_return != VOID) {
        errorHandle.printErrorOfReturnMatch(LEX_LINE);
    }
    if (grammar_meet_main) {
        ADD_MIDCODE(OpExit, "", "", "")
    } else if (addReturnMidcode == false) {
        ADD_MIDCODE(OpReturn, VOID, "", "")
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
    ADD_MIDCODE(OpScanf, LEX_LONA, "", "")
    PRINT_GET
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
        string strName = stringAnalyzer();
        ADD_MIDCODE(OpPrint, VOID, strName, "")
        if (TOKEN_TYPE == COMMA) {
            PRINT_GET
            string tmpValue;
            SymbolType tmpSymType = expressionAnalyzer(tmpValue);
            ADD_MIDCODE(OpPrint, tmpSymType, tmpValue, "")
        }
    } else {
        string tmpValue;
        SymbolType tmpSymType = expressionAnalyzer(tmpValue);
        ADD_MIDCODE(OpPrint, tmpSymType, tmpValue, "")
    }
    //CHECK_//GET(RPARENT, ")");
    CHECK_RPARENT
    ADD_MIDCODE(OpPrint, VOID, "nLine", "")
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
                auto *funcSym = new FuncSym(LEX_NAME, LEX_LONA, MAIN);
                symbolTable.insertFuncToGlobal(funcSym);
                ADD_MIDCODE(OpFunc, VOID, LEX_LONA, "")
                // ＜主函数＞
                PRINT_GET
                CHECK_GET(LPARENT, "( main");
                //CHECK_//GET(RPARENT, ") main");
                // 无参数函数定义
                CHECK_RPARENT
                CHECK_GET(LBRACE, "{ main");
                grammar_meet_main = true;
                compoundStatementAnalyzer();
                CHECK_GET(RBRACE, "} main");
                funcSym->setFuncLocalTable(symbolTable.localIdenTable);
                symbolTable.localIdenTable.clear();
                funcSym->funcSpace = grammar_var_offset;
                grammar_var_offset = 0;
                PRINT_MES(("<主函数>"))
                return;
            } else {
                // ＜无返回值函数定义＞  ::= void＜标识符＞
                auto *funcSym = new FuncSym(lexer.strToken, LEX_LONA, VOID);
                ADD_MIDCODE(OpFunc, VOID, lexer.lexToken.content_p, "")
                PRINT_GET
                // '('＜参数表＞')''{'＜复合语句＞'}'
                CHECK_GET(LPARENT, "( main");
                parameterTableAnalyzer(*funcSym, true);
                if (!errorHandle.checkDupFuncDefine(funcSym->lowerName, LEX_LINE)) {
                    symbolTable.insertFuncToGlobal(funcSym);
                }
                CHECK_RPARENT
                CHECK_GET(LBRACE, "{ main");
                compoundStatementAnalyzer();
                CHECK_GET(RBRACE, "} main");
                funcSym->setFuncLocalTable(symbolTable.localIdenTable);
                symbolTable.localIdenTable.clear();
                funcSym->funcSpace = grammar_var_offset;
                grammar_var_offset = 0;
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
    bool hasFunc = !errorHandle.checkUndefFuncCall(lower_name, LEX_LINE);
    CHECK_GET(LPARENT, "func call (");
    vector<VarSym> vectorVar = symbolTable.getFuncParams(lower_name);
    valParaTableAnalyzer(vectorVar);
    //CHECK_//GET(RPARENT, "func call )");
    // 有/无参数函数调用
    CHECK_RPARENT
    if (hasFunc) {
        ADD_MIDCODE(OpCall, lower_name, "", "")
        if (funcType != VOID) {
            PRINT_MES(("<有返回值函数调用语句>"))
        } else {
            PRINT_MES(("<无返回值函数调用语句>"))
        }
    }
    return funcType;
}

/*＜值参数表＞   ::= ＜表达式＞{,＜表达式＞}｜＜空＞*/
void GrammarAnalyzer::valParaTableAnalyzer(vector<VarSym> &vectorVar) {
    vector<SymbolType> referParamsList;
    if (TOKEN_TYPE == RPARENT) {
        // ＜空＞
    } else {
        string exp_str;
        int exp_int;
        SymbolType st = expressionAnalyzer(exp_str, exp_int);
        if (st != VOID) {
            if (st == INTVAR) {
                referParamsList.push_back(INT);
            } else if (st == CHARVAR) {
                referParamsList.push_back(CHAR);
            } else {
                referParamsList.push_back(st);
            }
            if (st == INT || st == CHAR) {
                ADD_MIDCODE(OpPaVal, st, exp_int, "")
            } else {
                ADD_MIDCODE(OpPaVal, st == INTVAR ? INT : CHAR, exp_str, "")
            }
        }
        while (TOKEN_TYPE == COMMA) {
            PRINT_GET
            st = expressionAnalyzer(exp_str, exp_int);
            if (st != VOID) {
                if (st == INTVAR) {
                    referParamsList.push_back(INT);
                } else if (st == CHARVAR) {
                    referParamsList.push_back(CHAR);
                } else {
                    referParamsList.push_back(st);
                }
                if (st == INT || st == CHAR) {
                    ADD_MIDCODE(OpPaVal, st, exp_int, "")
                } else {
                    ADD_MIDCODE(OpPaVal, st == INTVAR ? INT : CHAR, exp_str, "")
                }
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
string GrammarAnalyzer::stringAnalyzer() {
    static int labelIndex = -1;
    stringData tmpStringData;
    tmpStringData.label = "str_" + to_string(++labelIndex);
    tmpStringData.content = move(lexer.strToken);
    irCode.addStringData(tmpStringData);
    PRINT_GET
    PRINT_MES(("<字符串>"))
    return tmpStringData.label;
}

/*＜情况语句＞  ::=  switch ‘(’＜表达式＞‘)’ ‘{’＜情况表＞＜缺省＞‘}’*/
void GrammarAnalyzer::switchStatementAnalyzer() {
    static int switch_index = -1;
    string switch_end_label = "switch_" + to_string(++switch_index);
    CHECK_GET(SWITCHTK, "switch");
    CHECK_GET(LPARENT, "func call (");
    string exp_str;
    int exp_int;
    SymbolType switchType = expressionAnalyzer(exp_str, exp_int);
    SymbolType switch_type = switchType;
    if (switchType == INTVAR) {
        switchType = INT;
    } else if (switchType == CHARVAR) {
        switchType = CHAR;
    }
    //CHECK_//GET(RPARENT, "func call )");
    CHECK_RPARENT
    CHECK_GET(LBRACE, "{ main");
    situationSwitchAnalyzer(switchType, switch_type, exp_str, exp_int, switch_end_label);
    if (!defaultSwitchAnalyzer()) {
        // switch语句中，缺少<缺省>语句。
        errorHandle.printErrorLine('p', LEX_LINE);
    }
    CHECK_GET(RBRACE, "} main");
    ADD_MIDCODE(OpLabel, switch_end_label, "", "")
    PRINT_MES(("<情况语句>"))
}

/*＜情况表＞   ::=  ＜情况子语句＞{＜情况子语句＞} */
void GrammarAnalyzer::situationSwitchAnalyzer(SymbolType switchType, SymbolType isIntOrChar,
                                              string &exp1_str, int &exp1_int, string &end_label) {
    do {
        int switchConstant;
        // ＜情况子语句＞  ::=  case＜常量＞：＜语句＞
        CHECK_GET(CASETK, "CASE");
        if (constantAnalyzer(switchConstant) != switchType) {
            errorHandle.printErrorOfUnMatchConst(LEX_LINE);
        }
        CHECK_GET(COLON, "CASE");
        string case_str = genTmpVar_and_insert(INT);
        if (isIntOrChar == INT || isIntOrChar == CHAR) {
            ADD_MIDCODE(OpEQL, case_str, exp1_int, switchConstant);
        } else {
            ADD_MIDCODE(OpEQL, case_str, exp1_str, switchConstant);
        }
        string next_case = genLabel();
        ADD_MIDCODE(OpBEZ, next_case, case_str, "")
        statementAnalyzer();
        ADD_MIDCODE(OpJmp, end_label, "", "")
        PRINT_MES(("<情况子语句>"))
        ADD_MIDCODE(OpLabel, next_case, "", "")
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

void GrammarAnalyzer::readToRightBrack_and_log_error() {
    // 数组初始化个数不匹配
    PRINT_G_ERR('n')
    while (TOKEN_TYPE != RBRACE) {
        if (TOKEN_TYPE == LBRACE) {
            while (TOKEN_TYPE != RBRACE) {
                PRINT_GET
            }
        }
        PRINT_GET
    }
    PRINT_GET
}

void GrammarAnalyzer::endConstOrVarDeclaration() {
    symbolTable.endGlobalIdenSymbol();
    grammar_var_offset = 0;
    ADD_MIDCODE(OpJMain, "", "", "")
}

int GrammarAnalyzer::getNeedSpace(int level, int length1, int length2) {
    int need;
    switch (level) {
        case 0:
            need = 1;
            break;
        case 1:
            need = length1;
            break;
        case 2:
            need = length1 * length2;
            break;
        default:;
    }
    return need;
}

string GrammarAnalyzer::genTmpVar_and_insert(SymbolType symType) {
    static int index = -1;
    string ans = "tmp_" + to_string(++index);
    ans = symbolTable.insertTempSymToLocal(ans, symType, grammar_var_offset);
    grammar_var_offset++;
    // cout << "/////////////////// gen tmp named " << ans << " and offset is " << to_string(grammar_var_offset) << endl;
    return ans;
}

string GrammarAnalyzer::genLabel() {
    static int label_index = -1;
    string ans = "label_" + to_string(++label_index);
    return ans;
}