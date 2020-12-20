//
// Created by dlely on 2020/11/11.
//

#include "IRCodeManager.h"

#include <utility>
#include "stdlib.h"

#define INIT_THREE_ADD_CODE \
setObj(0, rrd);\
setObj(1, rrs);\
setObj(2, rrt);

using namespace std;

IRCodeManager::IRCodeManager(ofstream &tmpCodeFile) : ircodeFile(tmpCodeFile) {
}

IRCodeManager &IRCodeManager::getInstance(ofstream &tmpCodeFile) {
    static IRCodeManager initializer(tmpCodeFile);
    return initializer;
}

void IRCodeManager::addThreeAddCode(ThreeAddCode *code) {
    if (!threeAddCodeList.empty()) {
        threeAddCodeList.back()->setNext(code);
    }
    threeAddCodeList.push_back(code);
}

list<ThreeAddCode *> IRCodeManager::getThreeAddCodeList() {
    return threeAddCodeList;
}

list<stringData> IRCodeManager::getStringDataList() {
    return stringDataList;
}

void IRCodeManager::addStringData(stringData pronString) {
    stringDataList.push_back(pronString);
}

bool IRCodeManager::updateLastCode(string &ans) {
//    ThreeAddCode *p_code = threeAddCodeList.back();
//    p_code->setObj(0, ans);
    return true;
}

string ThreeAddCode::toString() {
    string ans = opTypeToString(op);
    for (int i = 0; i < 3; i++) {
        if (!obj[i].str.empty()) {
            ans.append("\t\t[" + branchToString(obj[i].branch) + obj[i].str + "]");
        }
    }
    return ans;
}

string ThreeAddCode::toMidString() {
    string ans;
    switch (op) {
        case OpPLUS:
            ans = obj[0].str + " = " + obj[1].str + " + " + obj[2].str;
            break;
        case OpMINU:
            ans = obj[0].str + " = " + obj[1].str + " - " + obj[2].str;
            break;
        case OpMULT:
            ans = obj[0].str + " = " + obj[1].str + " * " + obj[2].str;
            break;
        case OpDIV:
            ans = obj[0].str + " = " + obj[1].str + " / " + obj[2].str;
            break;
        case OpASSIGN:
            ans = obj[0].str + " = " + obj[1].str;
            break;
        case OpScanf:
            ans = "scanf(" + obj[0].str + ")";
            break;
        case OpPrint:
            ans = "print(" + obj[1].str + ")";
            break;
        case OpConst:
            ans = "const " + obj[0].str + " " + obj[1].str + " = " + obj[2].str;
            break;
        case OpVar:
            ans = "var " + obj[0].str + " " + obj[1].str;
            break;
        case OpFunc:
            ans = obj[0].str + " " + obj[1].str + "()";
            break;
        case OpJMain:
            ans = "call main()";
            break;
        case OpExit:
            ans = "GOTO end";
            break;
        case OpAssArray:
            ans = obj[0].str + "[" + obj[1].str + "] = " + obj[2].str;
            break;
        case OpGetArray:
            ans = obj[0].str + " = " + obj[1].str + "[" + obj[2].str + "]";
            break;
        case OpLSS:
            ans = obj[1].str + " < " + obj[2].str;
            break;
        case OpLEQ:
            ans = obj[1].str + " <= " + obj[2].str;
            break;
        case OpGRE:
            ans = obj[1].str + " > " + obj[2].str;
            break;
        case OpGEQ:
            ans = obj[1].str + " >= " + obj[2].str;
            break;
        case OpEQL:
            ans = obj[1].str + " == " + obj[2].str;
            break;
        case OpNEQ:
            ans = obj[1].str + " != " + obj[2].str;
            break;
        case OpBEZ:
            ans = "BZ " + obj[0].str;
            break;
        case OpBNEZ:
            ans = "BNZ " + obj[0].str;
            break;
        case OpJmp:
            ans = "GOTO " + obj[0].str;
            break;
        case OpLabel:
            ans = obj[0].str + ":";
            break;
        case OpParam:
            ans = "para " + obj[0].str + " " + obj[1].str;
            break;
        case OpPaVal:
            ans = "push " + obj[1].str;
            break;
        case OpCall:
            ans = "call " + obj[0].str;
            break;
        case OpReturn:
            ans = "ret " + obj[0].str;
            break;
        case OpRetVar:
            ans = obj[0].str + " = RET";
            break;
    }
    return ans;
}

void ThreeAddCode::setObj(int index, string pronStr) {
    obj[index].str = pronStr;
    obj[index].branch = 0;
}

void ThreeAddCode::setObj(int index, SymbolType pronSym) {
    if (pronSym == CHAR) {
        obj[index].str = "Char";
    } else if (pronSym == INT) {
        obj[index].str = "Int";
    } else {
        obj[index].str = "Void";
    }
    obj[index].type = pronSym;
    obj[index].branch = 1;
}

void ThreeAddCode::setObj(int index, int pronNum) {
    obj[index].str = to_string(pronNum);
    obj[index].num = pronNum;
    obj[index].branch = 2;
}

ThreeAddCode::ThreeAddCode(OperatorType op) : op(op) {
}

ThreeAddCode::ThreeAddCode(OperatorType op, string rrd, string rrs, string rrt) : op(op) {
    INIT_THREE_ADD_CODE
}

ThreeAddCode::ThreeAddCode(OperatorType op, SymbolType rrd, string rrs, string rrt) : op(op) {
    INIT_THREE_ADD_CODE
}

ThreeAddCode::ThreeAddCode(OperatorType op, SymbolType rrd, string rrs, int rrt) : op(op) {
    INIT_THREE_ADD_CODE
}

ThreeAddCode::ThreeAddCode(OperatorType op, SymbolType rrd, int rrs, string rrt) : op(op) {
    INIT_THREE_ADD_CODE
}

ThreeAddCode::ThreeAddCode(OperatorType op, string rrd, string rrs, int rrt) : op(op) {
    INIT_THREE_ADD_CODE
}

ThreeAddCode::ThreeAddCode(OperatorType op, string rrd, int rrs, string rrt) : op(op) {
    INIT_THREE_ADD_CODE
}

ThreeAddCode::ThreeAddCode(OperatorType op, string rrd, int rrs, int rrt) : op(op) {
    INIT_THREE_ADD_CODE
}

ThreeAddCode::ThreeAddCode(OperatorType op, string rrd, SymbolType rrs, string rrt) : op(op) {
    INIT_THREE_ADD_CODE
}

ThreeAddCode::ThreeAddCode(OperatorType op, int rrd, SymbolType rrs, string rrt) : op(op) {
    INIT_THREE_ADD_CODE
}

void ThreeAddCode::setNext(ThreeAddCode *pron_next) {
    p_next = pron_next;
}

ThreeAddCode *ThreeAddCode::getNext() {
    return p_next;
}

string ThreeAddCode::opTypeToString(OperatorType op) {
    return operatorString[op];
}

string ThreeAddCode::branchToString(int index) {
    if (index == 0) {
        return "string  : ";
    } else if (index == 1) {
        return "symType : ";
    } else {
        return "int/char: ";
    }
}

ThreeAddCode *ThreeAddCode::copy() {
    auto *p = new ThreeAddCode(this->op);
    p->obj[0].operator=(this->obj[0]);
    p->obj[1].operator=(this->obj[1]);
    p->obj[2].operator=(this->obj[2]);
    return p;
}
