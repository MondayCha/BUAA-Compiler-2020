# 2020错误处理阶段设计文档

## 1. 需求分析

### 1.1 问题描述

请根据给定的文法设计并实现错误处理程序，能诊察出常见的语法和语义错误，进行错误局部化处理，并输出错误信息。

- **诊察出常见的语法和语义错误**：按照题目所给的错误类型设计对应的处理程序
- **错误局部化处理**：如何最小化错误的影响？如何提升性能？



## 2. 初步思路

- **关于文法**：在语法分析中，我对文法进行了修改，使得不同产生式之间没有交集。因此错误处理只需要在对应的递归调用子程序中增加判断条件就可以了。
- **表达式类型**：表达式有char和int两种类型，因此我将表达式、项、因子的返回值都设置为对应的类型，一旦某个char类型参与了运算就转换为int，为空时则返回void，与符号表管理所用的枚举类一致。
- **符号表管理**：注意到在本次作业中，常量、变量定义只能在主函数之前或函数块开头，因此对于错误处理来说最多只有一张全局符号表和一张局部符号表，因此我们可以简化符号表类的设计。



## 3. 模块设计

### 3.1 错误处理类设计

```c++
class ErrorHandle {
private:
    explicit ErrorHandle(SymbolTable &symbolTable, ofstream &errorFile);   // Singleton Pattern
    SymbolTable &symbolTable;
    ofstream &errorFile;
public:
    static ErrorHandle &getInstance(SymbolTable &symbolTable, ofstream &errorFile);

    void printErrorLine(char errorCode, int &lineNum);

    ...

    void checkChangeConstIden(string &lowerName, int &lineNum);
};
```

使用静态成员实现单例模式，对外提供 `getInstance()`方法来访问这个对象。坦言之，目前我的错误处理类设计并不理想——我的错误处理类究竟要承担什么功能？比如对于非法符号类型的错误，对于字符类型我会将符号传入对应的错误处理方法，用`checkIllegalCharReturnNull`判断是否存在非法字符；但字符串类型我就直接在`Lexer`中处理了，因为本身也要逐个扫描，仅仅调用`printErrorLine`打印错误信息。所以结果上这里互相调用非常散乱，可以认为就是面向过程了……在之后的错误处理上，我想我会重新规划这一设计。

错误处理类提供的方法如下：
| 名称                                                         | 描述                                                   |
| ------------------------------------------------------------ | ------------------------------------------------------ |
| `void printErrorLine(char errorCode, int &lineNum);`         | 打印错误码和行数                                       |
| `void printErrorLine(char errorCode, int &lineNum, bool &findError);` | 重载，如果在本行已经打印过一次，就不再打印错误码和行数 |
| `void printErrorOfReturnMatch(int &lineNum);`                | 打印返回值错误                                         |
| `bool checkIllegalCharReturnNull(char &content, int &lineNum, bool &findError);` | 检查字符含有非法符号                                   |
| `bool checkDupIdenDefine(string &lowerName, int &lineNum);`  | 检查重复定义的标识符                                   |
| `bool checkDupFuncDefine(string &lowerName, int &lineNum);`  | 检查重复定义的函数                                     |
| `bool checkUndefFuncCall(string &lowerName, int &lineNum);`  | 检查未定义函数调用                                     |
| `bool checkArrayIndexNotInt(SymbolType expType, int &lineNum);` | 检查数组下标不为Int类型                                |
| `bool checkUndefIdenRefer(string &lowerName, int &lineNum);` | 检查未定义标识符引用                                   |
| `void printErrorOfNoReturnMatch(int &lineNum);`              | 打印无返回值的函数存在不匹配的return语句错误           |
| `void printErrorOfUnMatchConst(int &lineNum);`               | 打印<常量>类型不一致错误                               |
| `void checkChangeConstIden(string &lowerName, int &lineNum);` | 检查改变常量的值错误                                   |

选择在词法分析程序中直接调用`printErrorLine`还是对应的`ErrorHandle`检查的主要依据是：是否在多处需要分析此类型错误。



### 3.2 符号表类设计

```c++
class SymbolTable {
private:
    explicit SymbolTable();

    friend class GrammarAnalyzer;

    map<string, FuncSym> globalFuncTable;
    map<string, Symbol> globalIdenTable;
    map<string, Symbol> localIdenTable;
public:
    static SymbolTable &getInstance();

    ...

    vector<VarSym> getFuncParams(string &lower_name);
};
```

符号表类同样采用单例模式，内部储存了三个哈希表：

| 名称              | 描述                                                         |
| ----------------- | ------------------------------------------------------------ |
| `globalFuncTable` | 函数表，单独储存，添加时需要判断是否与表内元素或`globalIdenTable`重名 |
| `globalIdenTable` | 全局常量、变量表，在读取完声明声明后用`move`从`localIdenTable`中得到 |
| `localIdenTable`  | 当前的常量、变量、参数表                                     |

对于符号，则是在父类`Symbol`中增加基本的类型（int, void ,char）、属性（func, var, const），在子类中添加各自特有的属性。为了方便比较同名符号，保存小写转换后的名字属性。

```c++
// 父类（没设成抽象类……）
class Symbol {
private:
    string name;
    string lowerName;
    SymbolAtt symbolAtt;
    SymbolType symbolType;
public:
    Symbol(string &pronName, string &pronLowerName, SymbolAtt pronAtt, SymbolType pronType);
};

// 变量类符号
class VarSym : public Symbol {
private:
    int level;		// 存储维数
    int length1;	// 维数1
    int length2;	// 维数2
public:
    VarSym(string &pronName, string &pronLowerName, SymbolType pronType, int plevel, int plength1, int plength2);
};

// 函数类符号
class FuncSym : public Symbol {
private:
    vector<VarSym> parameters;	// 函数形参
public:
    FuncSym(string &pronName, string &pronLowerName, SymbolType pronType);
};

// 常量类符号
class ConSym : public Symbol {
private:
    int content;				// 常量初始值
public:
    ConSym(string &pronName, string &pronLowerName, SymbolType pronType, int pronContent);
};
```

符号表类对外提供的方法如下：

| 名称              | 描述                                                         |
| ----------------- | ------------------------------------------------------------ |
| `bool hasIdenName(string &name);`  | 当前符号表中是否存在该名称符号（用于检查重名定义） |
| `bool hasIdenNameIncludeGlobal(string &name);`  | 当前符号表以及全局符号表中是否存在该名称符号（用于检查未定义调用） |
| `bool hasFuncName(string &name);`  | 是否存在该名称函数 |
| `void insertSymbolToLocal(const Symbol &symbol);`  | 将符号插入到当前的局部符号表 |
| `void insertFuncToGlobal(const FuncSym &funcSym);`  | 插入函数到全局函数表 |
| `void endGlobalIdenSymbol();`  | 将局部符号表`move`为全局符号表 |
| `void endLocalIdenSymbol();`  | 清空局部符号表 |
| `SymbolType convertTypeCode(TypeCode typeCode);`  | 将`TypeCode`转换为对应的`SymbolType` |
| `SymbolType getFuncType(string &lower_name);`  | 获取函数类型 |
| `SymbolType getIdenType(string &lower_name);`  | 获取符号类型 |
| `SymbolAtt getIdenAtt(string &lower_name);`  | 获取符号属性 |
| `vector<VarSym> getFuncParams(string &lower_name);`  | 返回函数的参数表向量 |



## 4. 设计修改情况

在编码后，我进行了如下调整：

1. 增加一键开关输出的宏，在上一次的语法分析作业中，我将打印词法、语法分析结果用PRINT_GET、PRINT_MES等宏封装起来，而此次作业则增加`ifdef`语句，方便开关输出：

   ```c++
   #ifdef CLOSE_GRAMMER_OUTPUT
   #define PRINT_GET lexer.getNextToken();
   #define PRINT_MES(Message)
   #else
   #define PRINT_GET outFile << lexer.analyzerResult << endl;lexer.getNextToken();
   #define PRINT_MES(Message) outFile << Message << endl;
   #endif
   ```

2. 对于数组类型个数不匹配，我选择了一旦发现问题，立刻读取到“}”末尾的跳读方式，进行相应的错误局部化处理；然而在处理函数的参数表时则不能这样做：因为函数参数表结束的标志：“)”可能缺少，就会导致跳读无法正常结束，这也导致了我课下测试Testfile 5 TLE的原因。

   而为了修改，我最后选择了先保存函数的实参表，然后从函数符号类`FuncSym`中获取向量`vector<VarSym> parameters`，先比较二者长度，再逐个检查类型是否一致。

   ```c++
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
   ```

其他部分基本与当初的设计思路一致。



## 5. 个人总结

上次写语法分析设计文档时夸下的海口果然一个都没实现，这次错误处理开头又把语法分析的遗留问题重新整理了一遍。另外这次错误处理的课下测试点感觉也不强，比如我之前对于形如`while(1)`的语句部分为空语句，但缺少分号的情况就无法正确处理，结果也通过了全部测试点……不知道是为了避免歧义不做考察还是挖坑，但这意味着我的错误处理代码还是有可能存在其他问题的。
