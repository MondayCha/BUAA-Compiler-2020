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
    if (threeAddCodeList.size() != 0) {
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

string ThreeAddCode::toString() {
    string ans = opTypeToString(op);
    for (int i = 0; i < 3; i++) {
        if (obj[i].str != "") {
            ans.append("\t\t[" + branchToString(obj[i].branch) + obj[i].str + "]");
        }
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
