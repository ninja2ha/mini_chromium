# 算数安全(运算&转换)

#### 值校验

##### [cr::CheckedNumeric\<Dst\>]() (*constexpr*)
>数值模板类，可检查算数结果是否越过目标(Dst)类型值域，常用于一些需要精确计算的场景，如缓冲区长度计算等。   
**NOTE**:  
该对象支持运算符操作，以及不同数值类型之间的转换。
* [头文件]()
```c++
#include "cr_base/numeric/checked_math.h"
```

* 相关函数

|函数|描述|
|:--|:--|
|[cr::IsValidForType<Dst>]()|检查值是否符合目标(Dst)类型值域|
|[cr::ValueOrDieForType]()|获取算数值，并检查值是否符合目标类型的值域，否则创建一个崩溃点|
|[cr::ValueOrDefaultForType]()|获取算数值，并检查值是否符合目标类型的值域，否则返回一个默认值|
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

* 样例
```c++
// unsigned值域:{0 ~ 4294967295}

// div右侧表达式(5u/1)的结果在unsigned值域里，
// 因此div的值是有效的。
cr::CheckedNumeric<unsigned> div = cr::CheckedDiv(5u, 1); // Valid value;
div++; // cr::CheckedNumeric 支持使用运算符操作

// sub右侧表达式(0u-1)的结果越过了unsigned值域最小值，
// 因此sub的值是无效的。
cr::CheckedNumeric<unsigned> sub = cr::CheckedSub(0u, 1); // Invalid value;
sub--; // sub在运算之前值已经失效了，因此该运算值还是无效的。

// 在计算sum右侧表达式(4294967295u+1-1)过程中越过了unsigned值域最大值， 
// 因此sub的值是无效的。
cr::CheckedNumeric<unsigned> sum = cr::CheckedAdd(4294967295u, 1, -1); // Invalid value;
```

___
#### 值收缩
##### [cr::ClampedNumeric\<Dst\>]() (*constexpr*)
>数值模板类，用于在算数结果越过目标(Dst)类型的值域时，使用目标(Dst)类型值域的最大或最小边界值来赋值，常用于坐标计算。  
**NOTE**:  
该对象支持运算符重载操作，不支持不同数值类型之间的转换。
* [头文件]()
```c++
#include "cr_base/numeric/clamped_math.h"
```

* 相关函数

|函数|描述|
|:--|:--|
|[cr::MakeClampedNum]()|创建一个`cr::MakeClampedNum`对象|
||*以下函数返回一个`cr::ClampedNumeric<T>`对象，函数支持多元，`T`从元组里获取，优先级按照double，float，ui64，i64，ui32，i32...*|
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

* 样例
```c++
// unsigned值域:{0 ~ 4294967295}

// div右侧表达式(5u/1)的值在unsigned值域里。
// 因此div取的是运算出来的值5。
cr::ClampedNumeric<unsigned> div = cr::ClampDiv(5u, 1); // div = 5;

// sub右侧表达式(0u-1)的值越过了unsigned值域最小值，
// 因此sub取的是unsigned值域最小值0。
cr::ClampedNumeric<unsigned> sub = cr::ClampSub(0u, 1); // sub = 0;

// sum右侧表达式(4294967295u+1-1) 
// 在第一次运算过程中，其结果越过unsigned值域最大值, 因此sum = 4294967295。
// 第二次运算-1, 其表达式为4294967295 - 1，在unsigned值域内，因此sum =  4294967294
cr::ClampedNumeric<unsigned> sum = cr::ClampAdd(4294967295u, 1, -1); // sum = 4294967294;
```

___
#### 值安全转换
##### [cr::StrictNumeric\<Dst\>]() (*constexpr*)
>数值模板类，在编译期检测右值(Src)类型的值域是否在左值(Dst)类型值域的区间里，否则不通过编译。
* [头文件]()
```c++
#include "cr_base/numeric/safe_conversion.h"
```

* 用例
```c++
// char{-128 ~ 127}, int {-2147483648 ~ 2147483647}
cr::StrictNumeric<int> chr_2_int = (char)1;  // 编译通过
cr::StrictNumeric<char> int_2_char = (int)1; // 编译失败
```

##### [cr::IsTypeInRangeForNumericType\<Dst,Src\>]()
>数值类型值范围比较模板，用来比较目标类型的任意值是否都在源类型的数值范围里。
* [头文件]()
```c++
#include "cr_base/numeric/safe_conversion.h"
```

* 用例
```c++
constexpr bool v1 = cr::IsTypeInRangeForNumericType<int, char>::value; // true
constexpr bool v1 = cr::IsTypeInRangeForNumericType<char, int>::value; // false
```

##### 相关函数

|函数|描述|
|:--|:--|
|[cr::AsSigned]()|无符号数值类型=>有符号数值类型(static_cast<>的封装)|
|[cr::AsUnsigned]()|有符号数值类型=>无符号数值类型(static_cast<>的封装)|
|[cr::CheckedCast]()|数值类型强转换(static_cast<>的封装)，当右值(Src)转左值(Dst)过程中发生数值溢出时会创建一个崩溃点|
|[cr::StrictCast]()|数值类型强转换(static_cast<>的封装)，编译过程中，当左值(DST)类型无法容下右值(Src)类型的任何一个值时会编译错误，例如把数值从`unsigned int`转到`int`。|
|[cr::SaturatedCast]()|数值类型强转换(static_cast<>的封装)，当右值(Src)转左值(Dst)过程中发生数值溢出时会返回左值(Dst)类型的边界值(最大或最小)|
|||
|[cr::SafeUnsignedAbs]()|获取数值的绝对值，返回无符号数值类型|
|[cr::IsValueInRangeForNumericType\<Dst\>]()|判断值是否在目标(Dst)类型值范围里。|
|[cr::IsValueNegative]()|判断数值是否小于`0`，对于无符号的类型，始终返回`false`|
|||
|[cr::ClampFloor]()|浮点数向下取整(1.1=>1，1.5=>1)，然后转到左值(Dst)类型，当右值(Src)转左值(Dst)过程中发生数值溢出，则返回左值(Dst)类型的边界值(最大或最小)|
|[cr::ClampCeil]()|浮点数向上取整(1.1=>2，1.5=>2),然后转到左值(Dst)类型，当右值(Src)转左值(Dst)过程中发生数值溢出，则返回左值(Dst)类型的边界值(最大或最小)|
|[cr::ClampRound]()|浮点数四舍五入(1.1=>1，1.5=>2),然后转到左值(Dst)类型，当右值(Src)转左值(Dst)过程中发生数值溢出，则返回左值(Dst)类型的边界值(最大或最小)|
|||
|[cr::MakeStrictNum]()|创建对象`cr::StrictNumeric<>`|

