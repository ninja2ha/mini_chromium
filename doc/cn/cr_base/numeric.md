# 安全算数

### 值校验

###### [cr::CheckedNumeric\<T\>]()
>校验数值模板类，用于储存数值以及检测值是否存在类型溢出，可直接使用运算符。
```c++
/* 头文件 */
#include "cr_base/numeric/checked_math.h"
```
* 相关函数

|函数|描述|
|:--|:--|
|[cr::IsValidForType]()|校验值是否符合其类型|
|[cr::ValueOrDieForType]()|从`cr::CheckedNumeric<T>`获取值并且校验值是否符合其类型，否则创建一个崩溃点|
|[cr::ValueOrDefaultForType]()|从`cr::CheckedNumeric<T>`获取值并且校验值是否符合其类型，否则返回一个默认值|
|[cr::MakeCheckedNum]()|创建一个`cr::CheckedNumeric<T>`对象|
|[cr::CheckMax]()|从最大值，返回到`cr::CheckedNumeric<T>`对象|
|[cr::CheckMin]()|从最小值，返回到`cr::CheckedNumeric<T>`对象|
|[cr::CheckAdd]()|从左到右，依次求和, 支持多元，返回到`cr::CheckedNumeric<T>`对象|
|[cr::CheckSub]()|从左到右，依次求差, 支持多元，返回到`cr::CheckedNumeric<T>`对象|
|[cr::CheckMul]()|从左到右，依次求积, 支持多元，返回到`cr::CheckedNumeric<T>`对象|
|[cr::CheckDiv]()|从左到右，依次求除数, 支持多元，返回到`cr::CheckedNumeric<T>`对象|
|[cr::CheckMod]()|从左到右，依次求余, 支持多元，返回到`cr::CheckedNumeric<T>`对象|
|[cr::CheckLsh]()|从左到右，依次做左移运算, 支持多元，返回到`cr::CheckedNumeric<T>`对象|
|[cr::CheckRsh]()|从左到右，依次做右移运算, 支持多元，返回到`cr::CheckedNumeric<T>`对象|
|[cr::CheckAnd]()|从左到右，依次做与运算, 支持多元，返回到`cr::CheckedNumeric<T>`对象|
|[cr::CheckOr]()|从左到右，依次做或运算, 支持多元，返回到`cr::CheckedNumeric<T>`对象|
|[cr::CheckXor]()|从左到右，依次做异或运算, 支持多元，返回到`cr::CheckedNumeric<T>`对象|

##### 