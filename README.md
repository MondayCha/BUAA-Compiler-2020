# 2020语法分析阶段设计文档

## 1. 需求分析

### 1.1 问题描述

请根据给定的文法设计并实现语法分析程序，能基于上次作业的词法分析程序所识别出的单词，识别出各类语法成分。输入输出及处理要求如下：

1. 需按文法规则，用递归子程序法对文法中定义的所有种语法成分进行分析；**（所用方法：递归子程序法，可以在课本中找到进一步的案例）**

2. 为了方便进行自动评测，输入的被编译源文件统一命名为testfile.txt（注意不要写错文件名）；输出的结果文件统一命名为output.txt（注意不要写错文件名）；结果文件中包含如下两种信息：

   - 按词法分析识别单词的顺序，按行输出每个单词的信息（要求同词法分析作业，对于预读的情况不能输出）。**（如何处理预读？保证输出顺序正确？不能简单在词法分析中输出上一次的结果了）**

   - 在文法中高亮显示（见2020文法定义）的语法分析成分分析结束前，另起一行输出当前语法成分的名字，形如“<常量说明>”（注：未要求输出的语法成分仍需要分析）**（注意点：需要输出中文，而Windows命令行默认使用GBK编码，可能存在问题）**

### 1.2 输入形式

testfile.txt中的符合文法要求的测试程序。

### 1.3 输出形式

按如上要求将语法分析结果输出至output.txt中，中文字符的编码格式要求是UTF-8。

### 1.4 特别提醒

1. 本次作业只考核对正确程序的处理，但需要为今后可能出现的错误情况预留接口。**（错误处理需要在下一次的作业实现，我感觉这个我可能会写很久……）**

2. 当前要求的输出只是为了便于评测，完成编译器中无需出现这些信息，请设计为方便打开/关闭这些输出的方案。**（我的处理方式：用宏语句包裹输出语句，方便之后关闭）**



## 2. 初步思路

在编码之前，我是这样规划我的语法分析部分设计的：

1. 使用一遍过的方式，将词法分析读一个词的操作封装成一个方法，在语法分析程序中调用这个方法，就可以读取到下一个Token。

2. 修改文法。可以注意到这次的文法较为复杂，存在需要回溯或者预读的情况。为此我的初步解决方案是修改文法，使得不同产生式之间没有交集。但这种做法的一大问题便在于错误处理——我使用了很多条件语句对输入进行细分，而这在遇到非标准输入时就很可能引发未知的问题。

3. 调整输出。在上一次的词法分析中，每当识别出一个Token，就会将其输出到output文件上，但这次的语法分析器就不能这样做，一个典型的例子就是：

   ```c
   ＜程序＞::= ［＜常量说明＞］［＜变量说明＞］{＜有返回值函数定义＞|＜无返回值函数定义＞}＜主函数＞
   ＜变量说明＞  ::= ＜变量定义＞;{＜变量定义＞;}
   ＜有返回值函数定义＞  ::=  ＜声明头部＞'('＜参数表＞')' '{'＜复合语句＞'}'
   ```
   在这里变量说明和有返回值函数定义有着相同的声明头部（int 和char），在识别到函数的"("之前是无法判断变量说明是否已经结束的。所以这里我选择了在读取变量定义时如果发现是函数定义，就进行输出顺序的调整。

6. 在这次的作业中虽然不要求符号表，但是需要为函数建立一个简单的属性表，从而正确识别出有返回值和无返回值函数调用语句。



## 3. 模块设计

### 3.1 语法分析器类设计

```c++
class GrammarAnalyzer {
private:
    explicit GrammarAnalyzer(Lexer &pronLexer, SymbolTable &pronSymbol, std::ofstream &pronFile);

    Lexer &lexer;
    SymbolTable &symbolTable;
    ofstream &outFile;
    ///////// analyze grammar //////////

public:
    static GrammarAnalyzer &getInstance(Lexer &pronLexer, SymbolTable &pronSymbol, std::ofstream &pronOutFile);

    
    void analyzeGrammar();

    ///////// 出于篇幅限制，省略了其他递归调用的分析子程序 //////////
    void programAnalyzer();
    ...
    void defaultSwitchAnalyzer();
};
```

使用静态成员实现单例模式，对外提供 `getInstance()`方法来访问这个对象。同时修改上次作业词法分析类`Lexer.h`中`Lexer`的定义，把每次扫描的结果存到Token中，将`GrammarAnalyzer`设置为`friend class`，从而让语法分析程序可以直接调用词法分析器的结果。`SymbolTable`与`Lexer`相似，由`GrammarAnalyzer`管理，在本次作业中仅存储函数是否为有返回值函数这一性质。



### 3.2 语法分析方法框架

进入语法分析程序首先调用`programAnalyzer()`，分析＜程序＞语法成分，在`programAnalyzer()`结尾则输出语法分析结果，其他的分析程序与之类似。

```c++
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
    // {＜有返回值函数定义＞|＜无返回值函数定义＞}＜主函数＞
    functionDeclarationAnalyzer();
    outFile << "<程序>" << endl;
}
```

在这里我用宏封装了在语法分析程序中对词法分析程序调用的几个主要操作，例如：

```c++
#define GET_TOKEN lexer.getNextToken()
#define PRINT_LEX {outFile << lexer.analyzerResult;}
#define PRINT_GET {outFile << lexer.analyzerResult;lexer.getNextToken();}
#define CHECK_GET(A, B) {\
if(TOKEN_TYPE != A){\
cerr << B << endl;\
}\
PRINT_GET;\
}
```

其中`GET_TOKEN`就是调用`lexer.getNextToken()`方法，把获取的Token保存到`lexer`类的属性中；`PRINT_LEX`则是输出词法分析结果；`PRINT_GET`是输出词法分析结果后调用`GET_TOKEN`获取下一个输入；`CHECK_GET(A, B)`则在`PRINT_GET`的基础上增加了对Token类型的校验，方便调试（但不具有错误处理功能）。

借助这些宏，本次作业的可读性有一定提升，之后也能比较方便地关闭这些输出。



## 4. 设计修改情况

在编码后，我进行了如下调整：

1. 为之后的错误处理考虑，我增加对数组类型赋值、定义语句中，括号嵌套层数和元素个数的校验。
   ```c++
string GrammarAnalyzer::varDefinitionAnalyzer(TypeCode typeCode) {
       int level = 0, length1 = 0, length2 = 0;
       if (TOKEN_TYPE == LBRACK) {
           // 保存数组类型中的层数、行列数
           arrayVarAnalyzer(level, length1, length2);
       }
       if (TOKEN_TYPE == ASSIGN) {
           PRINT_GET;
           // 读取值并进行校验
           varInitAnalyzer(level, length1, length2, (typeCode == INTTK));
           return "<变量定义及初始化>\n";
       } else if (TOKEN_TYPE == COMMA) {
           ...
           return "<变量定义无初始化>\n";
       }
       return "<变量定义无初始化>\n";
}
   ```
   这里的`arrayVarAnalyzer(int &level, int &l1, int &l2)`使用引用从而实现对多个值的修改，进而可以在`varInitAnalyzer`中进行检查。

2. 对于变量说明和有返回值函数定义有着相同的声明头部（int 和char）的情况，使用条件语句特判：

   ```c++
   /*＜变量说明＞  ::= ＜变量定义＞;{＜变量定义＞;}
    * 需要判断第一次遇到有返回值函数的情况*/
void GrammarAnalyzer::varDeclarationAnalyzer() {
       // 变量定义
       string tmp_identity, tmp_output1, tmp_output2, tmp_output3;
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
           tmp_identity = lexer.lexToken.content_p;
           lexer.getNextToken();
           // ＜有返回值函数定义＞
           if (TOKEN_TYPE == LPARENT) {
               if (loopTime != 0) {
                   outFile << "<变量说明>" << endl;
               }
               outFile << tmp_output1;
               outFile << tmp_output2;
               symbolTable.addFunc(tmp_identity, true);
               meetFunc = true;
               speReValFuncDefAnalyzer();
           } else {
               // ＜变量定义＞
               ...
           }
           if (meetFunc) {
               return;
           }
           ++loopTime;
       } while (TYPE_IDEN);
       outFile << "<变量说明>" << endl;
   }
   ```
   
   这里的设计思路比较混乱，比如在半途增加了一个`loopTime`用来记录变量说明的循环次数，如果是第一次循环就遇到了有返回值函数调用语句，那么就不输出“<变量说明>”，这都是在Debug时的惨痛经历……好在目前可以运行，之后应该会重构。

其他部分基本与当初的设计思路一致。



## 5. 个人总结

这次的语法分析在之前的词法分析基础上进行修改。在这次我周围不少同学因为中文的输出而产生问题，比如我的一个舍友在奇数个中文时会显示乱码，还有群里出现的最后两个汉字乱码的问题……这个貌似只能解释称Windows的锅；另外还有因为“\n \r”的各种问题，如果不是自己过滤输入中的空白字符应该就不会发生。

目前让我比较担忧的就是之后的错误处理和符号表管理能否少出点Bug了，这两天也会陆续重新调整下各语法分析函数的功能顺序，以实现与后续功能需求更平滑的衔接。
