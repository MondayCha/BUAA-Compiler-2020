#include <iostream>
#include "include/Lexer.h"
#include "include/GrammarAnalyzer.h"

using namespace std;

int main() {
    string inputFile = "testfile.txt";
    string outputFile = "output.txt";
    ifstream fileIn(inputFile);
    if (!fileIn.is_open()) {
        cout << "can not find testfile.txt" << endl;
        return 1;
    }
    ofstream fileOut(outputFile);
    Lexer scanner = Lexer::getInstance(fileIn,fileOut);
    SymbolTable symbolTable = SymbolTable::getInstance();
    GrammarAnalyzer grammarAnalyzer = GrammarAnalyzer::getInstance(scanner,symbolTable,fileOut);
    grammarAnalyzer.analyzeGrammar();
    return 0;
}
