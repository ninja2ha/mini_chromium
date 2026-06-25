# 算数安全(运算&转换)

#### 数值校验

##### [cr::CheckedNumeric\<Dst\>]() (*constexpr*)
>数值模板类，用于检查算数结果是否越过目标(Dst)类型值域，常用于一些需要精确计算的场景，如缓冲区长度计算等。   
*NOTE*:  
此类支持运算符操作，以及不同数值类型之间的转换。
* [头文件]()
```c++
#include "cr_base/numeric/checked_math.h"
```

* 相关函数

|函数|描述|
|:--|:--|
|[cr::IsValidForType\<Dst\>]()|检查`值`是否在`R(Dst)`中|
|[cr::ValueOrDieForType\<Dst\>]()|检查`值`是否在`R(Dst)`中，是则返回该`值`，否则产生一个软中断|
|[cr::ValueOrDefaultForType\<Dst\>]()|检查`值`是否在`R(Dst)`中，是则返回该`值`，否则返回一个`默认值`|
|[cr::MakeCheckedNum]()|创建一个`cr::CheckedNumeric<>`对象|
||*以下函数返回`cr::CheckedNumeric<T>`，函数支持多元，`T`从元组里获取，优先级按照double，float，ui64，i64，ui32，i32...*|
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
// R(unsigned) = {0 ~ 4294967295}

// div: 右侧表达式(5u/1)=5，属于R(unsigned)，因此div的值是有效的。
cr::CheckedNumeric<unsigned> div = cr::CheckedDiv(5u, 1); // Valid value;
div++; // cr::CheckedNumeric 支持使用运算符操作

// sub: 右侧表达式(0u-1)=-1，不属于R(unsigned)，因此sub的值是无效的。
cr::CheckedNumeric<unsigned> sub = cr::CheckedSub(0u, 1); // Invalid value;
sub--; // sub在运算之前值已经失效了，因此该运算值还是无效的。

// sum: 右侧表达式(4294967295u+1-1), 在(4294967295u+1)后已经越过Max(R(unsigned))， 
//      因此sub的值是无效的。
cr::CheckedNumeric<unsigned> sum = cr::CheckedAdd(4294967295u, 1, -1); // Invalid value;
```

___
#### 数值收缩
##### [cr::ClampedNumeric\<Dst\>]() (*constexpr*)
>数值模板类，用于在算数结果越过`R(Dst)`时，使用`R(Dst)`的极限值(`Min(R(Dst))`或  
`Max(R(Dst))`)来赋值，常用于坐标计算。  
*NOTE*:  
该对象支持运算符重载操作，不支持不同数值类型之间的转换。
* [头文件]()
```c++
#include "cr_base/numeric/clamped_math.h"
```

* 相关函数

|函数|描述|
|:--|:--|
|[cr::MakeClampedNum]()|创建一个`cr::MakeClampedNum`对象|
||*以下函数返回`cr::ClampedNumeric<T>`，函数支持多元，`T`从元组里获取，优先级按照double，float，ui64，i64，ui32，i32...*|
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
// R(unsigned) = {0 ~ 4294967295}

// div: 右侧表达式(5u/1)=5‌，5∈ R(unsigned), 因此div=5
cr::ClampedNumeric<unsigned> div = cr::ClampDiv(5u, 1); // div = 5;

// sub: 右侧表达式(0u-1)=-1， -1 < Min(R(unsigned))，
//      因此sub = Min(R(unsigned)) = 0
cr::ClampedNumeric<unsigned> sub = cr::ClampSub(0u, 1); // sub = 0;

// sum： 右侧表达式(4294967295u+1-1) 
//       - 第一次运算(4294967295u+1)>Max(R(unsigned))，
//         因此sum = Max(R(unsigned)) = 4294967295
//       - 第二次运算sum-1, 其表达式为(4294967295-1)，在unsigned值域内，
//         因此sum = 4294967294
cr::ClampedNumeric<unsigned> sum = cr::ClampAdd(4294967295u, 1, -1); // sum = 4294967294;
```

___
#### 数值安全转换
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
>数值类型值域比较模板，用来比较目标(Dst)类型的任意值是否都在源(Src)类型的值域里。
* [头文件]()
```c++
#include "cr_base/numeric/safe_conversion.h"
```

* 用例
```c++
constexpr bool v1 = cr::IsTypeInRangeForNumericType<int, char>::value; // true
constexpr bool v1 = cr::IsTypeInRangeForNumericType<char, int>::value; // false
```

**相关函数**

|函数|说明|
|:--|:--|
|[cr::AsSigned\<Dst\>]()|无符号数值=>有符号数值|
|[cr::AsUnsigned\<Dst\>]()|有符号数值=>无符号数值|
|[cr::CheckedCast\<Dst\>]()|数值转换，当`右值‌∉R(Dst)`，产生一个软中断|
|[cr::StrictCast\<Dst,Src\>]()|数值转换，在编译过程中，‌当`R(‌Src)∉‌R(Dst)`，编译则不通过。</br>例：`char c = StrictCast<char,int>(1)`|
|[cr::SaturatedCast\<Dst\>]()|数值转换，当`右值‌∉R(Dst)`，返回`R(Dst)`极限值。</br>小于`Min(R(Dst))`时返回`Min(R(Dst))`</br>大于`Max(R(Dst))`时返回`Max(R(Dst))`|
|||
|[cr::SafeUnsignedAbs]()|获取绝对值，返回无符号数值类型|
|[cr::IsValueInRangeForNumericType\<Dst\>]()|判断指定值是否在`R(Dst)`里。|
|[cr::IsValueNegative]()|判断数值是否小于`0`，对于无符号的类型，始终返回`false`|
|||
|[cr::ClampFloor\<Dst\>]()|浮点数向下取整(1.1=>1，1.5=>1)，然后转到目标(Dst)类型，当`结果∉‌R(Dst)`，取`R(Dst)`极限值。</br>小于`Min(R(Dst))`时返回`Min(R(Dst))`</br>大于`Max(R(Dst))`时返回`Max(R(Dst))`|
|[cr::ClampCeil\<Dst\>]()|浮点数向上取整(1.1=>2，1.5=>2),然后转到目标(Dst)类型，当`结果∉‌R(Dst)`，取`R(Dst)`极限值。</br>小于`Min(R(Dst))`时返回`Min(R(Dst))`</br>大于`Max(R(Dst))`时返回`Max(R(Dst))`|
|[cr::ClampRound\<Dst\>]()|浮点数四舍五入(1.1=>1，1.5=>2),然后转到目标(Dst)类型，当`结果∉‌R(Dst)`，取`R(Dst)`极限值。</br>小于`Min(R(Dst))`时返回`Min(R(Dst))`</br>大于`Max(R(Dst))`时返回`Max(R(Dst))`|
|||
|[cr::MakeStrictNum]()|创建对象`cr::StrictNumeric<>`|

