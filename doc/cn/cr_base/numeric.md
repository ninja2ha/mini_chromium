# 安全算数

### 校验值

###### [cr::CheckedNumeric\<T\>]()
>检测数值模板类，用于储存运算结果，以及检查结果超出是否目标类型值范围的问题。  
**NOTE**: 支持运算符操作。

```c++
/* 头文件 */
#include "cr_base/numeric/checked_math.h"
```

* 相关函数

|函数|描述|
|:--|:--|
|[cr::IsValidForType]()|检查值是否符合目标类型的值范围|
|[cr::ValueOrDieForType]()|检查值是否符合目标类型的值范围，否则创建一个崩溃点|
|[cr::ValueOrDefaultForType]()|检查值是否符合目标类型的值范围，否则返回一个默认值|
|[cr::MakeCheckedNum]()|创建一个`cr::CheckedNumeric<T>`对象|
||*以下函数返回一个`cr::CheckedNumeric<T>`对象，函数支持多元，`T`取决于元类型最大值最大的那个*|
|[cr::CheckMax]()|取最大值|
|[cr::CheckMin]()|取最小值|
|[cr::CheckAdd]()|从左到右，依次求和|
|[cr::CheckSub]()|从左到右，依次求差|
|[cr::CheckMul]()|从左到右，依次求积|
|[cr::CheckDiv]()|从左到右，依次求除数|
|[cr::CheckMod]()|从左到右，依次求余,|
|[cr::CheckLsh]()|从左到右，依次做左移运算|
|[cr::CheckRsh]()|从左到右，依次做右移运算|
|[cr::CheckAnd]()|从左到右，依次做与运算|
|[cr::CheckOr]()|从左到右，依次做或运算|
|[cr::CheckXor]()|从左到右，依次做异或运算|

### 收缩值
##### [cr::ClampedNumeric\<T\>]()
>数值收缩模板类，用于储存运算结果，以及当结果超过类型`T`能表达的数值范围时，则使用该类型的最大值或最小值来储存。  
**NOTE**: 支持运算符重载操作。

```c++
/* 头文件 */
#include "cr_base/numeric/clamped_math.h"
```

* 相关函数

|函数|描述|
|:--|:--|
|[cr::MakeClampedNum]()|创建一个`cr::MakeClampedNum`对象|
||*以下函数返回一个`cr::MakeClampedNum<T>`对象，函数支持多元，`T`优先使用无符号的元类型*|
|[cr::ClampMax]()|取最大值|
|[cr::ClampMin]()|取最小值|
|[cr::ClampAdd]()|从左到右，依次求和|
|[cr::ClampSub]()|从左到右，依次求差|
|[cr::ClampMul]()|从左到右，依次求积|
|[cr::ClampDiv]()|从左到右，依次求除数|
|[cr::ClampMod]()|从左到右，依次求余|
|[cr::ClampLsh]()|从左到右，依次做左移运算|
|[cr::ClampRsh]()|从左到右，依次做右移运算|
|[cr::ClampAnd]()|从左到右，依次做与运算|
|[cr::ClampOr]()|从左到右，依次做或运算|
|[cr::ClampXor]()|从左到右，依次做异或运算|
