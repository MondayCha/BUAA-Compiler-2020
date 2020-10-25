#include <iostream>
#include "include/Lexer.h"
#include "include/GrammarAnalyzer.h"
#include "include/ErrorHandle.h"

using namespace std;

int main() {
    string inputFile = "testfile.txt";
    string outputFile = "output.txt";
    string errorFile = "error.txt";
    ifstream fileIn(inputFile);
    if (!fileIn.is_open()) {
        cout << "can not find testfile.txt" << endl;
        return 1;
    }
    ofstream fileOut(outputFile);
    ofstream errorOut(errorFile);
    SymbolTable symbolTable = SymbolTable::getInstance();
    ErrorHandle errorHandle = ErrorHandle::getInstance(symbolTable, errorOut);
    Lexer scanner = Lexer::getInstance(errorHandle,fileIn, fileOut);
    GrammarAnalyzer grammarAnalyzer = GrammarAnalyzer::getInstance(scanner, symbolTable, errorHandle, fileOut);
    grammarAnalyzer.analyzeGrammar();
    return 0;
}
