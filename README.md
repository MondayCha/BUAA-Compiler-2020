# 2020代码优化阶段说明文档

### 1. 常数传播

- 对于形如`x=1+2`的表达式，直接计算出结果
- 对于形如`x=y+2`的表达式，采用`addi`指令
- 常量不为其分配MIPS存储空间，在计算时从符号表找到对应的值加入运算
- 对于包含0的四元式，采用`$0`代替0进行计算
- 条件跳转语句为两个常数进行比较时，改为无条件跳转



### 2. 表达式计算

将乘数、被除数为2的整数次幂的情况转换为左移和逻辑右移。其中除法需要进行分支跳转，代价从10降低到了5。判断被除数是否为2的整数幂则采用移位运算。

```c++
bool isImm2Gre = value2 > 0;
unsigned int tmpDivValueAbs = isImm2Gre ? value2 : -value2;
bool isTwoPower = (tmpDivValueAbs & (tmpDivValueAbs - 1)) == 0;
if (value2 == 1) {
  MIPS_CODE("move\t" << rd1s << ",\t" << rs2s)
} else if (value2 == 0 || !isTwoPower) {
  MIPS_CODE("li\t\t" << rt3s << ",\t" << to_string(value2))
    MIPS_CODE("div\t\t" << rs2s << ",\t" << rt3s)
    MIPS_CODE("mflo\t" << rd1s)
} else {
  string labelDiv = "l_div_" + to_string(divLabelOffset++);
  if (isImm2Gre) {
    MIPS_CODE("move\t$t0,\t" << rs2s)
  } else {
    MIPS_CODE("sub\t\t$t0,\t$0,\t" << rs2s)
  }
  MIPS_CODE("bgtz\t$t0,\t" << labelDiv)
  // 获得一个整数的二进制位数
  unsigned int binaryOfValue = 1;
  while ((tmpDivValueAbs >> binaryOfValue) > 0) {
    binaryOfValue++;
  }
  MIPS_CODE("addiu\t$t0,\t$t0,\t" << to_string(tmpDivValueAbs - 1))
    MIPS_PRINT(labelDiv << ":")
    MIPS_CODE("sra\t\t" << rd1s << ",\t$t0,\t" << to_string(binaryOfValue - 1))
}
```



### 3. 窥孔优化

对于赋值语句的右支为临时变量的情况，可以将赋值语句合并到上一句语句。



### 4. 临时寄存器分配

临时变量在表达式计算过程中产生，仅存在声明和使用，生命周期在基本快内，因此采用FIFO分配方式，将临时变量分配到临时寄存器。

其中对于Switch语句，表达式的结果需要多次使用，进行特判处理。



### 5. 全局寄存器分配

采用引用计数，统计函数内的非临时变量的变量声明和引用次数，运用优先队列的数据结构保存结果，将队列中排名靠前的分配对应的全局寄存器。

```c++
////////////////// sReg ////////////////////
int sRegNum = 0;
while (!funcPtr->varUseCountQueue.empty() && sRegNum < 8) {
  string varName = funcPtr->varUseCountQueue.top().name;
  funcPtr->varUseCountQueue.pop();
  isGlobal = false;
  Symbol *p_symbol = symbolTable.getSymbolPtr(&funcPtr->funcLocalTable, varName, isGlobal);
  if (!isGlobal) {
    sRegBusy[sRegNum] = 1;
    sRegName[sRegNum] = varName;
    cout << "\t\t\t\t# " << "allocate " << sRegName[sRegNum] << " to $s" << to_string(sRegNum) << endl;
    sRegNum++;
  }
}
while (sRegNum < 8) {
  sRegBusy[sRegNum] = 0;
  sRegNum++;
}
////////////////////////////////////////////
```



### 6. While、For循环优化

由于优化后排名下降，故最终版本没有采用。



### 7. 死函数删除

从main函数开始广度优先搜索引用的函数，没有用到的函数则不生成对应的MIPS代码（虽然没用）。