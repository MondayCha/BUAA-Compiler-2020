# 2020词法分析阶段设计文档

<p align="right">18373580 李亦龙</p>
[TOC]

## 1. 需求分析

根据给定的文法设计并实现词法分析程序，从源程序中识别出单词，记录其单词类别和单词值。特别注意：

1. 读取的字符串要原样保留着便于输出，特别是数字，这里输出的并不是真正的单词值，其实是读入的字符串，单词值需另行记录。（不用处理前导零等情况，直接原样输出）
2. 本次作业只考核对正确程序的处理，但需要为今后可能出现的错误情况预留接口。在今后的错误处理作业中，需要输出错误的行号，在词法分析的时候注意记录该信息。（在进行词法分析时记录行号，为后续的模块做准备）
3. 单词的类别和单词值以及其他关注的信息，在词法分析阶段获取后，后续的分析阶段会使用，请注意记录；当前要求的输出只是为了便于评测，完成编译器中无需出现这些信息，请设计为方便打开/关闭这些输出的方案。（另外准备存储词法分析结果的容器或者输出词法分析结果的方式）





## 2. 初步思路

在编码之前，我是这样规划我的词法分析部分设计的：

1. 获取输入流，每次读取一个字符，每识别出一个Token就将结果输出
2. 考虑到之后还要添加语法分析、错误处理、语义分析等功能，将负责词法分析的程序封装为一个对象，通过函数调用词法分析方法，降低不同模块之间的耦合度
3. 第一次作业不需要错误处理，但应该留好进入错误处理的入口



## 3. 模块设计

### 3.1 词法分析器类设计

```c++
class Lexer {
private:
    Lexer(ifstream &inputFile, ofstream &outputFile);
    ifstream &inFile;
    ofstream &outFile;
public:
    static Lexer &getInstance(ifstream &inputFile, ofstream &outputFile);
    static string getTypeStr(TypeCode typeCode);
    bool getNextToken();
};
```

使用静态成员实现单例模式，对外提供 `getInstance()`方法来访问这个对象。在本次作业，将输入流和输出流传入对象，之后则可以移除输出流。



### 3.2 词法分析方法框架

以读入标识符的过程为例，说明`getNextToken()`工作原理：

```c++
if (isalund(c)) {
    do {
        token[pt++] = c;
        c = inFile.get();	// 获取下一个字符
    } while (isalund(c) || isdigit(c));
    token[pt] = '\0';		// 读取完毕
    strToken = token;
    string lowerToken = string();
    for (int i = 0; i < pt; ++i) {
        token[i] = char(tolower(token[i]));
    }
    lowerToken = token;		// 转换为小写
    auto iter = reservedWordMap.find(lowerToken);
    if (iter != reservedWordMap.end()) {
        typeCode = iter->second;	// 是保留字
    } else {
        typeCode = IDENFR;  // 与保留字不同，是标识符
    }
}
```



### 3.3 类别码存储方式

使用枚举值保存类别码：

```c++
enum TypeCode {
    IDENFR, ELSETK, MINU, ASSIGN,
    INTCON, SWITCHTK, MULT, SEMICN,
    CHARCON, CASETK, DIV, COMMA,
    STRCON, DEFAULTTK, LSS, LPARENT,
    CONSTTK, WHILETK, LEQ, RPARENT,
    INTTK, FORTK, GRE, LBRACK,
    CHARTK, SCANFTK, GEQ, RBRACK,
    VOIDTK, PRINTFTK, EQL, LBRACE,
    MAINTK, RETURNTK, NEQ, RBRACE,
    IFTK, PLUS, COLON, WRONG
};
```

同时建立根据类别码找对应输出字符的映射，只要用字符数组存储，将枚举值作为索引即可。



## 4. 设计修改情况

在编码后，我进行了如下调整：

1. 错误输出暂时不考虑，因为还不清楚最后的错误输出的距离处理方式，现阶段有的函数还没有做好适应，比如对于一个内容为空的字符串，在两个双引号之间没有内容，但目前我采用的是这种读取方式：

```c++
else if (c == '\"') {
    c = inFile.get();
    do {
        token[pt++] = c;	// 如果‘"’后紧跟一个‘"’，就会出错
        c = inFile.get();
    } while (c != '\"');
    token[pt] = '\0';
    strToken = token;
    typeCode = STRCON;
    c = inFile.get();
}
```

   类似的情况在我的代码中不一而足，不能简单使用else导出错误情况，因此错误处理就留到之后解决。

其他部分基本与当初的设计思路一致。



## 5. 总结

相比语法分析，词法分析的内容还算基础。这次借着这个机会学习了一些C++的知识，之后我也会努力完成编译作业。