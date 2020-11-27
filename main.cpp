#include <iostream>
#include "include/Lexer.h"
#include "include/GrammarAnalyzer.h"
#include "include/ErrorHandle.h"
#include "include/MipsTranslator.h"

using namespace std;

int main() {
    string inputFile = "testfile.txt";
    string outputFile = "output.txt";
    string errorFile = "error.txt";
    string ircodeFile = "irCode.txt";
    string mipsFile = "mips.txt";
    ifstream fileIn(inputFile);
    if (!fileIn.is_open()) {
        cout << "can not find testfile.txt" << endl;
        return 1;
    }
    ofstream fileOut(outputFile);
    ofstream errorOut(errorFile);
    ofstream ircodeOut(ircodeFile);
    ofstream mipsOut(mipsFile);
    SymbolTable symbolTable = SymbolTable::getInstance();
    ErrorHandle errorHandle = ErrorHandle::getInstance(symbolTable, errorOut);
    Lexer scanner = Lexer::getInstance(errorHandle, fileIn, fileOut);
    IRCodeManager irCode = IRCodeManager::getInstance(ircodeOut);
    GrammarAnalyzer grammarAnalyzer = GrammarAnalyzer::getInstance(scanner, symbolTable, errorHandle, irCode, fileOut);
    grammarAnalyzer.analyzeGrammar();
    MipsTranslator mipsTranslator = MipsTranslator::getInstance(mipsOut, irCode, symbolTable);
    mipsTranslator.translate();
    return 0;
}
