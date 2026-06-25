# 字符串

#### 字符串视图类

##### [cr::BasicStringPiece\<CharT\>]() *(constexpr)*
>字符串视图，避免字符串在传递过程中重复**申请/释放内存**以及**计算长度**。  
*NOTE*:  
在异步任务中作参数传递时，容易导致访问野指针，因此勿将该类型作为异步任务的参数。

* [头文件]()
```c++
#include "cr_base/strings/string_piece.h"
```

* 扩展定义

|类型|定义|说明|
|:---|:---|:---|
|cr::StringPiece|cr::BasicStringPiece\<char\>|8位字符串视图|
|cr::StringPiece16|cr::BasicStringPiece\<char16_t\>|16位字符串视图|
|cr::StringPiece32|cr::BasicStringPiece\<char32_t\>|32位字符串视图|
|cr::WStringPiece|cr::BasicStringPiece\<wchar_t\>|宽字符串视图|

* 嵌套类型

|类型|定义|
|:---|:---|
|size_type|size_t|
|traits_type|std::char_traits\<CharT\>|
|value_type|CharT|
|pointer|const CharT*|
|reference|const CharT&|
|const_reference|const CharT&|
|difference_type|std::ptrdiff_t|
|const_iterator|const CharT*|
|const_reverse_iterator|std::reverse_iterator<const_iterator>|

* 常量成员

|类型|成员|值|说明|
|:---|:---|:---|:---|
|size_type|npos|size_type(-1)|无效位置|

* 数据成员

|类型|成员|说明|
|:---|:---|:---|
|`const CharT*` |`ptr_`|字符串指针|
|`size_type` |`length_`|字符串长度|

* 函数成员

|成员|说明|
|:---|:---|
|[(constructor)]()|构造一个`BasicStringPiece`|
|[data]()|获取字符串指针, 返回数据成员`ptr_`|
|[size]()|获取字符串长度, 返回数据成员`length_`|
|[length]()|同上|
|[empty]()|检查字符串长度是否为`0`|
|[operator[]]()|获取指定位置的字符|
|[front]()|获取首个字符|
|[back]()|获取末尾字符|
|[remove_prefix]()|根据给定的长度，移除前缀字符串|
|[remove_suffix]()|根据给定的长度，移除末尾字符串|
|[operator std::basic_string]()|隐式转换`BasicStringPiece`到`std::basic_string`|
|[begin]()|返回指向头部的迭代器|
|[end]()|返回指向尾部的迭代器|
|[rbegin]()|返回指向头部的反向迭代器|
|[rend]()|返回指向尾部的反向迭代器|
|[max_size]()|获取字符串最大长度，实际返回的是数据成员`length_`|
|[capacity]()|获取字符串的容量，实际返回的是数据成员`length_`|
|[copy]()|拷贝当前字符串到给定的地址|
|[substr]()|截取子字符串视图|
|[compare]()|比较两个字符串|
|[find]()|查找子字符串位置|
|[rfind]()|查找子字符串最后出现的位置|
|[find_first_of]()|根据给定的字符串，查找其子字符出现的首个位置|
|[find_last_of]()|根据给定的字符串，查找其子字符出现的最后位置|
|[find_first_not_of]()|根据给定的字符串，查找其子字符缺席的首个位置|
|[find_last_not_of]()|根据给定的字符串，查找其子字符缺席的最后位置|

* 非成员函数

|函数|说明|
|:--|:--|
|[cr::MakeStringPiece]()|构造对象`cr::StringPiece`|
|[cr::MakeStringPiece16]()|构造对象`cr::StringPiece16`|
|[cr::MakeStringPiece32]()|构造对象`cr::StringPiece32`|
|[cr::MakeWStringPiece]()|构造对象`cr::WStringPiece`|


* 用例
```c++
std::string str = "foo";

cr::StringPiece str_view(str);         // 构造期间得到缓冲区地址，以及长度
const char* str_ptr = str_view.data(); // 获取str数据缓冲区
size_t str_len = str_view.length();    // 获取str数据长度

cr::StringPiece str_view2 = str_view;  // 直接复制属性内容，无需申请缓冲区，以及计算长度。

// 注意：str 被清空后, str_view, str_view2的属性内容就失效了，不应当被继续使用。
str.clear();
```

___
#### 字符串操作函数

##### 大小写转换
* [头文件]()
```c++
#include "cr_base/strings/string_util.h"
```

* 函数

|函数|描述|
|:--|:--|
|[cr::ToLowerASCII]()|ASCII字符/字符串转小写|
|[cr::ToUpperASCII]()|ASCII字符/字符串转大写|


##### 字符串比对
* [头文件]()
```c++
#include "cr_base/strings/string_util.h"
```

* 函数

|函数|描述|
|:--|:--|
|[cr::CompareCaseInsensitiveASCII]()|比较两个字符串，不区分ASCII字符大小写|
|[cr::EqualsCaseInsensitiveASCII]()|查看两个字符串内容是否一致，不区分ASCII字符大小写|

##### 字符串单例
* [头文件]()
```c++
#include "cr_base/strings/string_util.h"
```

* 函数

|函数|描述|
|:--|:--|
|[cr::EmptyString](/#)|返回一个空的`std::string`单例对象|
|[cr::EmptyString16](/#)|返回一个空的`std::u16string`单例对象|
|[cr::EmptyString32](/#)|返回一个空的`std::u32string`单例对象|
|[cr::EmptyWString](/#)|返回一个空的`std::wstring`单例对象|

##### 字符串 移除/替换/填充
* [头文件]()
```c++
#include "cr_base/strings/string_util.h"
```

* 函数

|函数|描述|
|:--|:--|
|[cr::RemoveChars]()|移除指定字符串|
|[cr::ReplaceChars]()|替换指定字符串|
|[cr::TrimString]()|根据字符表，裁剪字符串首尾|
|[cr::TrimWhitespaceASCII]()|裁剪字符串的首尾ASCII空白字符|
|[cr::TrimWhitespace]()|裁剪字符串首尾空白字符|
|[cr::ReplaceFirstSubstringAfterOffset]()|在指定位置后，替换首个子字符串|
|[cr::ReplaceSubstringsAfterOffset]()|在指定位置后，替换所有子字符串|
|[cr::ReplaceStringPlaceholders]()|替换字符`'$'`|
|[cr::WriteInto]()|以指定的尺寸，重置`std::basic_string`缓冲区。|

##### 字符串判断
* [头文件]()
```c++
#include "cr_base/strings/string_util.h"
```

* 函数

|函数|描述|
|:--|:--|
|[cr::ContainsOnlyChars]()|字符串是否仅含有字符表里的字符|
|[cr::IsStringUTF8]()|字符串是否为UTF-8编码，不允许有非字符出现(66个特定的代码点)|
|[cr::IsStringUTF8AllowingNoncharacters]()|字符串是否为UTF-8编码，允许非字符(66个特定的代码点)|
|[cr::IsStringASCII]()|字符串是否全为ASCII字符|
|[cr::LowerCaseEqualsASCII]()|宽字符串的ACSII小写格式是否与指定的字符串（通常是ASCII小写常量）一致|
|[cr::EqualsASCII]()|宽字符串是否与指定的字符串相同|
|[cr::StartsWith]()|字符串是否以指定的字符串开头，支持区分ASCII大小写|
|[cr::EndsWith]()|字符串是否以指定的字符串结尾，支持区分ASCII大小写|
|[cr::IsAsciiWhitespace]()|字符是否为ACSII空白字符|
|[cr::IsAsciiAlpha]()|字符是否为英文('a'-'z'、'A'-'Z')|
|[cr::IsAsciiUpper]()|字符是否为大写英文('A-'Z')|
|[cr::IsAsciiLower]()|字符是否为小写英文('a'-'z')|
|[cr::IsAsciiDigit]()|字符是否为数字('0'-'9')|
|[cr::IsAsciiPrintable]()|字符是否可以在控制台打印|
|[cr::IsAsciiHexDigit]()|字符是否为Hex数字('0'-'9'、'a'-'f'、'A'-'F')|

##### C字符串标准操作
* [头文件]()
```c++
#include "cr_base/strings/string_util.h"
```
* 拷贝函数

|函数|描述|
|:--|:--|
|[cr::strlcpy]()|拷贝字符串|
|[cr::wcslcpy]()|拷贝宽字符串|
|[cr::strdup]()|申请拷贝字符串，需要调用free方法释放|
|[cr::wcsdup]()|申请拷贝宽字符串，需要调用free方法释放|

* 格式化函数

|函数|描述|
|:--|:--|
|[cr::snprintf]()|字符串格式化输出|
|[cr::snwprintf]()|宽字符串格式化输出|

* `wchar_t`<=>`char16_t`|`char32_t`转换函数
> `wchar_t`在不同的平台占用的空间不一定相同，在Windows下占用2个字节，  
在其他POSIX平台可能就会占用4个字节，具体见宏[MINI_CHROMIUM_WCHAR_IS_UTF16]()定义。

|函数|平台|描述|
|:--|:--|:--|
|[cr::AsWide]()||WIN：将`char16_t*`转换成`wchar_t*`</br>POSIX：将`char32_t*`转换成`wchar_t*`|
|[cr::AsConstWide]()||WIN：将`const char16_t*`转换成`const wchar_t*`</br>POSIX：将`const char32_t*`转换成`const wchar_t*`|
|[cr::AsUnicode16]()|WIN|将`wchar_t*`转换成`char16_t*`|
|[cr::AsConstUnicode16]()|WIN|将`char16_t*`转换成`wchar_t*`|
|[cr::AsUnicode32]()|POSXI|将`wchar_t*`转换成`char32_t*`|
|[cr::AsConstUnicode32]()|POSIX|将`char32_t*`转换成`wchar_t*`|

##### 字符串连接
* [头文件]()
```c++
#include "cr_base/strings/string_cat.h"
```

* 函数

|函数|描述|
|:--|:--|
|[cr::StrCat]()|依次连接字符串组的所有内容|
|[cr::StrAppend]()|依次连接字符串组的所有内容，并且追加到指定的目标|
|[cr::JoinString]()|将字符串组的所有内容使用分隔符依次连接|

##### 字符串分割
* [头文件]()
```c++
#include "cr_base/strings/string_split.h"
```

* 函数

|函数|描述|
|:--|:--|
|[cr::SplitString]()|使用字符表分割字符串，返回`std::vector<std::basic_string>`|
|[cr::SplitStringPiece]()|使用字符表分割字符串视图，返回`std::vector<cr::BasicStringPiece>`，减少内存Allocate|
|[cr::SplitStringIntoKeyValuePairs]()|使用字符分割字符串，并根据键值的分隔符分离出键与值|
|[cr::SplitStringIntoKeyValuePairsUsingSubstr]()|使用字符串分割字符串，并根据键值的分隔符分离出键与值|
|[cr::SplitStringUsingSubstr]()|使用字符串分割字符串，返回`std::vector<std::basic_string>`|
|[cr::SplitStringPieceUsingSubstr]()|使用字符串分割字符串视图，返回`std::vector<std::BasicStringPiece>`，减少内存Allocate|

##### 字符串格式化
* [头文件]()
```c++
#include "cr_base/strings/stringprintf.h"
```

* 函数

|函数|描述|
|:--|:--|
|[cr::StringPrintf]()|字符串格式化输出，返回格式化后的字符串|
|[cr::SStringPrintf]()|字符串格式化输出，赋值到指定的`std::basic_string`对象，避免重复拷贝，返回对象引用。|

##### 字符串数值转换
* [头文件]()
```c++
#include "cr_base/strings/string_number_conversions.h"
```

* 函数

|函数|描述|
|:--|:--|
|[cr::NumberToString]()|数值转字符串`std::string`|
|[cr::NumberToString16]()|数值转字符串`std::u16string`|
|[cr::NumberToString32]()|数值转字符串`std::u32string`|
|[cr::NumberToWString]()|数值转字符串`std::wstring`|
|||
|[cr::StringToInt]()|字符串转`int`，要求值在`int`值域里|
|[cr::StringToUint]()|字符串转`unsigned int`，要求值在`unsigned int`值域里|
|[cr::StringToInt64]()|字符串转`int64_t`，要求值在`int64_t`值域里|
|[cr::StringToUint64]()|字符串转`uint64_t`，要求值在`uint64_t`值域里|
|[cr::StringToSizeT]()|字符串转`size_t`，要求值在`size_t`值域里|
|[cr::StringToDouble]()|字符串转`double`，要求值在`double`值域里|
|||
|[cr::HexEncode]()|字节组转HEX字符串, 例:`{0x90,0x91,0x92} => "909192"`|
|[cr::HexStringToInt]()|HEX字符串(-0x80000000 ~ 0x7FFFFFFF)转`int`，前缀`0x`可选|
|[cr::HexStringToUInt]()|HEX字符串(0x00000000 ~ 0xFFFFFFFF)转`unsigned int`，前缀`0x`可选|
|[cr::HexStringToInt64]()|HEX字符串(-0x8000000000000000 ~ 0x7FFFFFFFFFFFFFFF)转`int64_t`，前缀`0x`可选|
|[cr::HexStringToUInt64]()|HEX字符串(0x0000000000000000 ~ 0xFFFFFFFFFFFFFFFF)转`uint64_t`，前缀`0x`可选|
|[cr::HexStringToBytes]()|HEX字符串转字节组，储存到`std::vector<uint8_t>`，例:`"909192" => {0x90,0x91,0x92}`|
|[cr::HexStringToString]()|HEX字符串转字节组，储存到`std::string`，例:`"909192" => {0x90,0x91,0x92}`|
|[cr::HexStringToSpan]()|HEX字符串转字节组，储存到已经固定尺寸的字节组缓冲区，例:`"909192" => {0x90,0x91,0x92}`|
