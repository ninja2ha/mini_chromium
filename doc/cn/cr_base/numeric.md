# 算数安全

### 值校验

###### [cr::CheckedNumeric\<T\>]()
>数值模板类，用于检查算数结果是否越过目标类型值边界。  
**NOTE**: 该对象支持运算符操作，以及不同数值类型之间的转换。

```c++
/* 头文件 */
#include "cr_base/numeric/checked_math.h"

/* 
** [样例]
**   unsigned类型的值边界: 0-4294967295
**
**   sub右侧表达式(0u-1)的值越过了unsigned类型值最小边界，
**   因此sub的值是无效的，sub.IsValid() = false。
**
**   sum右侧表达式(4294967295u+1)的值越过了unsigned类型值最大边界。
**   因此sum的值是无效的，sum.IsValid() = false。
**
**   div右侧表达式(5u/1)的值在unsigned类型值边界范围里。
**   因此div的值是有效的，div.IsValid() = true。
*/
cr::CheckedNumeric<unsigned> sub = cr::CheckedSub(0u, 1); // Invalid value;
cr::CheckedNumeric<unsigned> sum = cr::CheckedSum(4294967295u, 1); // Invalid value;
cr::CheckedNumeric<unsigned> div = cr::CheckedDiv(5u, 1); // Valid value;
```

* 相关函数

|函数|描述|
|:--|:--|
|[cr::IsValidForType]()|检查值是否符合目标类型的值范围|
|[cr::ValueOrDieForType]()|获取算数值，并检查值是否符合目标类型的值范围，否则创建一个崩溃点|
|[cr::ValueOrDefaultForType]()|获取算数值，并检查值是否符合目标类型的值范围，否则返回一个默认值|
|[cr::MakeCheckedNum]()|创建一个`cr::CheckedNumeric<T>`对象|
||*以下函数返回一个`cr::CheckedNumeric<T>`对象，函数支持多元，`T`从元组里获取，优先级按照double，float，ui64，i64，ui32，i32...*|
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

### 值收缩
##### [cr::ClampedNumeric\<T\>]()
>数值模板类，用于在算数结果越过类型`T`能表达的数值边界时，使用该类型的最大或最小边界值来赋值。  
**NOTE**: 该对象支持运算符重载操作，不支持不同数值类型之间的转换。

```c++
/* 头文件 */
#include "cr_base/numeric/clamped_math.h"

/* 
** [样例]
**   unsigned类型的值边界: 0-4294967295
**
**   sub右侧表达式(0u-1)的值越过了unsigned类型值最小边界，
**   因此sub取的是unsigned能够表示最小的值0。
**
**   sum右侧表达式(4294967295u+1)的值越过了unsigned类型值最大边界。
**   因此sum取的是unsigned能够表示最大的值4294967295。
**
**   div右侧表达式(5u/1)的值在unsigned类型值边界范围里。
**   因此div取的是运算出来的值5。
*/
cr::ClampedNumeric<unsigned> sub = cr::ClampSub(0u, 1); // sub = 0;
cr::ClampedNumeric<unsigned> sum = cr::ClampSum(4294967295u, 1); // sum = 4294967295;
cr::ClampedNumeric<unsigned> div = cr::ClampDiv(5u, 1); // div = 5;
```

* 相关函数

|函数|描述|
|:--|:--|
|[cr::MakeClampedNum]()|创建一个`cr::MakeClampedNum`对象|
||*以下函数返回一个`cr::MakeClampedNum<T>`对象，函数支持多元，`T`从元组里获取，优先级按照double，float，ui64，i64，ui32，i32...*|
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
