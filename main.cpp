#include <iostream>
#include <fstream>
#include "lib/Lexer.h"

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
    Lexer scanner = Lexer::getInstance(fileIn, fileOut);
    while (scanner.getNextToken());
    return 0;
}
