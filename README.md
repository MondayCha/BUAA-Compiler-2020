# 2020代码生成阶段设计文档

## 1. 需求分析

### 1.1 问题描述

在词法分析、语法分析及错误处理作业的基础上，为编译器实现语义分析、代码生成功能。输入输出及处理要求如下：

- 需根据文法规则及语义约定，采用自顶向下的语法制导翻译技术，进行语义分析并生成目标代码（任选其一）；

- 对于选择生成**MIPS**的编译器，约定如下：

  完成编译器，将源文件（统一命名为testfile.txt）编译生成MIPS汇编并输出到文件（统一命名为mips.txt），具体要求包括：

  1. 需自行设计四元式中间代码，再从中间代码生成MIPS汇编，请设计实现输出中间代码的有关函数，本次作业不考核，后续会有优化前后中间代码的输出及评判
  2. 若选择此项任务，后续的作业需参加竞速排序，请提前预留代码优化有关的接口，并设计方便切换开启/关闭优化的模式
  3. 自行调试时，可使用Mars仿真器，提交到平台的编译器只需要能按统一的要求生成MIPS汇编代码文件即可。



## 2. 初步思路

- 代码生成阶段主要分为两部分，中间代码生成以及从中间代码转到MIPS代码。参考网上的资料后，我的四元式设计包括操作符以及三个封装后的操作数结构体；在转换为MIPS代码时，则使用`$t0 $t1 $t2$`三个寄存器。
- 对于变量定义和常量定义，可以直接放在符号表中，不需要生成四元式。
- 函数调用和函数定义时，参数的中间代码参考课程组提供的中间代码规范，函数定义时先进行声明，之后一个个传入形参参数；函数调用时先压入参数，之后再生成调用函数的中间代码。



## 3. 模块设计

### 3.1 中间代码设计

```c++
class ThreeAddCode {
private:
    OperatorType op;
    GoalObject obj[3];
    ThreeAddCode *p_next = nullptr;
};

struct GoalObject {
    int branch;
    string str; // lower
    SymbolType type;
    int num;
};
```

操作数为一个结构体，第一个属性`branch`标记了该结构体所指向的内容，`branch`为0时意为该操作数为字符串形式（变量名、标签名等）；`branch`为1时说明是类型（`CHAR, INT, VOID`）；`branch`为2时表示立即数。同时在生成这三种类型的操作数时，都将`str`属性的值进行了初始化，方便输出中间代码。附上各运算符以及其含义：

| 运算符 | 含义 |
| ---- | ---- |
|`OpPLUS`| `+` |
|`OpMINU`| `-` |
|`OpMULT`| `*` |
|`OpDIV`| `/` |
|`OpASSIGN`| 赋值语句 |
|`OpScanf`| 读语句 |
|`OpPrint`| 写语句 |
|`OpConst`| 常量定义（以及初始化） |
|`OpVar`| 变量定义 |
|`OpFunc`| 函数声明 |
|`OpJMain`| 跳转到主函数（用在全局变量及初始化后） |
|`OpExit`| 调用系统调用的结束 |
|`OpAssArray`| 数组赋值 |
|`OpGetArray`| 数组取值 |
|`OpLSS`| `<` |
|`OpLEQ`| `<=` |
|`OpGRE`| `>` |
|`OpGEQ`| `>=` |
|`OpEQL`| `==` |
|`OpNEQ`| `!=` |
|`OpBEZ`| 操作数1为0时跳转 |
|`OpBNEZ`| 操作数1不为0时跳转 |
|`OpJmp`| 无条件跳转语句 |
|`OpLabel`| 生成标签 |
|`OpParam`| 函数形参 |
|`OpPaVal`| 函数实参 |
|`OpCall`| 函数调用 |
|`OpReturn`| 函数返回 |
|`OpRetVar `| 将返回值存储到操作数1 |

示例函数：

```c++
int seed[3] = {19971231, 19981013, 1000000007};
int i = 0;

void main(){
    int tmp = 0;
    while(i<1000){
        tmp = tmp + 100000 * 100000;
        tmp = tmp - tmp / 100000 * 100000;
        i = i + 1;
    }
    printf(tmp);
    return ;
}
```

中间代码示例：

```
VarDef     [symType : Int]       [string  : seed]      [int/char: 3]
AssArr    [string  : seed]      [int/char: 0]     [int/char: 19971231]
AssArr    [string  : seed]      [int/char: 1]     [int/char: 19981013]
AssArr    [string  : seed]      [int/char: 2]     [int/char: 1000000007]
VarDef    [symType : Int]       [string  : i]     [int/char: 1]
ASSIGN    [string  : i]     [int/char: 0]
JMain 
FunDef    [symType : Void]      [string  : main]
VarDef    [symType : Int]       [string  : tmp]       [int/char: 1]
ASSIGN    [string  : tmp]       [int/char: 0]
Label:    [string  : label_0]
cd LSS    [string  : tmp_0]     [string  : i]     [int/char: 1000]
bez       [string  : label_1]       [string  : tmp_0]
MULT      [string  : tmp_1]     [int/char: 100000]    [int/char: 100000]
PLUS      [string  : tmp_2]     [string  : tmp]       [string  : tmp_1]
ASSIGN    [string  : tmp]       [string  : tmp_2]
DIV       [string  : tmp_3]     [string  : tmp]       [int/char: 100000]
MULT      [string  : tmp_4]     [string  : tmp_3]     [int/char: 100000]
MINU      [string  : tmp_5]     [string  : tmp]       [string  : tmp_4]
ASSIGN    [string  : tmp]       [string  : tmp_5]
PLUS      [string  : tmp_6]     [string  : i]     [int/char: 1]
ASSIGN    [string  : i]     [string  : tmp_6]
Jmp to    [string  : label_0]
Label:    [string  : label_1]
Print     [symType : Int]       [string  : tmp]
Print     [symType : Void]      [string  : nLine]
Exit  
Exit  每个操作数同样输出了类型，方便debug。
```

而对于函数的参数空间分配，将局部变量存入`sp`指针下的栈；将全局变量存入`gp`指针下的栈。偏移量在符号表中就已经存储了。



## 4. 修改情况

我目前采用的四元式包括运算符、三个操作数以及一个指向下一条四元式的指针。事实上在写代码优化时我对这个架构非常后悔，目前在从四元式转换到MIPS代码时，对于每个变量和函数都需要依靠`str`的值去查表，如果可以在操作数中增加一个指向符号对象的指针，那么开销就会小很多……然而现在要改已经来不及了。

再加上最近其他课程的DDL也纷至沓来，其实我现在想想，代码优化才是编译最有挑战性的地方，但我前期却花了大量时间在递归下降子程序分析的重复劳动中，但是下周又要进行理论课考试……之后尽量做些代码优化吧。