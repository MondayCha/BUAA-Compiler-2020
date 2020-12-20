#include <iostream>
#include "include/Lexer.h"
#include "include/GrammarAnalyzer.h"
#include "include/ErrorHandle.h"
#include "include/MipsTranslator.h"
#include "include/ActiveOptimize.h"

using namespace std;

int main() {
    // FileStream
    string inputFile = "testfile.txt";
    string outputFile = "output.txt";
    string errorFile = "error.txt";
    string ircodeFile = "irCode.txt";
    string mipsFile = "mips.txt";
    // can not find testfile.txt
    ifstream fileIn(inputFile);
    if (!fileIn.is_open()) {
        cout << "can not find testfile.txt" << endl;
        return 1;
    }
    // out stream
    ofstream fileOut(outputFile);
    ofstream errorOut(errorFile);
    ofstream ircodeOut(ircodeFile);
    ofstream mipsOut(mipsFile);
    // class
    SymbolTable symbolTable = SymbolTable::getInstance();
    ErrorHandle errorHandle = ErrorHandle::getInstance(symbolTable, errorOut);
    Lexer scanner = Lexer::getInstance(errorHandle, fileIn, fileOut);
    IRCodeManager irCode = IRCodeManager::getInstance(ircodeOut);
    ActiveOptimize activeOptimize = ActiveOptimize::getInstance(symbolTable,irCode);
    GrammarAnalyzer grammarAnalyzer = GrammarAnalyzer::getInstance(scanner, symbolTable, errorHandle, irCode,
                                                                   activeOptimize, fileOut);
    grammarAnalyzer.analyzeGrammar();
    activeOptimize.optimizeCode();
    MipsTranslator mipsTranslator = MipsTranslator::getInstance(mipsOut, irCode, activeOptimize, symbolTable);
    mipsTranslator.translate();
    return 0;
}
