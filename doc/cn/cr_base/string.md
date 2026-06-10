# 字符串

#### 字符串视图类

###### [cr::BasicStringPiece\<CharT\>]() 
>用于快速访问字符串，避免在传递过程中内存Allocate，重复计算长度。  
**NOTE**: 在异步任务中用作参数传递，可能会导致访问野指针！
``` c++
/* 头文件 */
#include "cr_base/string/string_piece.h"

/* 定义 */
typedef cr::BasicStringPiece<char> cr::StringPiece;       // 8位字符串
typedef cr::BasicStringPiece<char16_t> cr::StringPiece16; // 16位字符串
typedef cr::BasicStringPiece<char32_t> cr::StringPiece32; // 32位字符串
typedef cr::BasicStringPiece<wchar> cr::WStringPiece;     // 宽字符串
```
___
#### 字符串操作函数

##### 大小写转换
```c++
/* 头文件 */
#include "cr_base/string/string_util.h"
```
|函数|描述|
|:--|:--|
|[cr::ToLowerASCII]()|ASCII字符/字符串转小写|
|[cr::ToUpperASCII]()|ASCII字符/字符串转大写|


##### 字符串比对
```c++
/* 头文件 */
#include "cr_base/string/string_util.h"
```
|函数|描述|
|:--|:--|
|[cr::CompareCaseInsensitiveASCII]()|比较两个字符串，不区分ASCII字符大小写|
|[cr::EqualsCaseInsensitiveASCII]()|查看两个字符串内容是否一致，不区分ASCII字符大小写|

##### 字符串单例
```c++
/* 头文件 */
#include "cr_base/string/string_util.h"
```
|函数|描述|
|:--|:--|
|[cr::EmptyString](/#)|返回一个空的`std::string`单例对象|
|[cr::EmptyString16](/#)|返回一个空的`std::u16string`单例对象|
|[cr::EmptyString32](/#)|返回一个空的`std::u32string`单例对象|
|[cr::EmptyWString](/#)|返回一个空的`std::wstring`单例对象|

##### 字符串 移除/替换/填充
```c++
/* 头文件 */
#include "cr_base/string/string_util.h"
```
|函数|描述|
|:--|:--|
|[cr::RemoveChars]()|移除指定的子字符串|
|[cr::ReplaceChars]()|替换指定子字符串|
|[cr::TrimString]()|根据字符表，裁剪字符串首尾|
|[cr::TrimWhitespaceASCII]()|裁剪字符串的首尾ASCII空白字符|
|[cr::TrimWhitespace]()|裁剪字符串首尾空白字符|
|[cr::ReplaceFirstSubstringAfterOffset]()|在指定位置后，替换首个子字符串|
|[cr::ReplaceSubstringsAfterOffset]()|在指定位置后，替换所有子字符串|
|[cr::ReplaceStringPlaceholders]()|替换字符`'$'`|
|[cr::WriteInto]()|以指定的尺寸，重置`std::basic_string`缓冲区。|

#### 字符串判断
```c++
/* 头文件 */
#include "cr_base/string/string_util.h"
```
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
|[cr::IsAsciiDigit]()|字符是否为数值('0'-'9')|
|[cr::IsAsciiPrintable]()|字符是否可以在控制台打印|
|[cr::IsAsciiHexDigit]()|字符是否为Hex数值('0'-'9'、'a'-'f'、'A'-'F')|

#### C-Style标准操作
```c++
/* 头文件 */
#include "cr_base/string/string_util.h"
```
* 拷贝

|函数|描述|
|:--|:--|
|[cr::strlcpy]()|拷贝字符串|
|[cr::wcslcpy]()|拷贝宽字符串|
|[cr::strdup]()|申请拷贝字符串，需要调用free方法释放|
|[cr::wcsdup]()|申请拷贝宽字符串，需要调用free方法释放|

* 格式化

|函数|描述|
|:--|:--|
|[cr::vsnprintf]()|字符串格式化输出|
|[cr::vsnwprintf]()|宽字符串格式化输出|
|[cr::snprintf]()|字符串格式化输出|
|[cr::snwprintf]()|宽字符串格式化输出|

* `wchar_t`类型转换 
> `wchar_t`在不同的平台占用的空间不一定相同，在Windows下占用2个字节，  
在其他POSIX平台可能就会占用4个字节，具体见宏[MINI_CHROMIUM_WCHAR_IS_UTF16]()。

|函数|平台|描述|
|:--|:--|:--|
|[cr::AsWide]()||WIN：将`char16_t*`转换成`wchar_t*`</br>POSIX：将`char32_t*`转换成`wchar_t*`|
|[cr::AsConstWide]()||WIN：将`const char16_t*`转换成`const wchar_t*`</br>POSIX：将`const char32_t*`转换成`const wchar_t*`|
|[cr::AsUnicode16]()|WIN|将`wchar_t*`转换成`char16_t*`|
|[cr::AsConstUnicode16]()|WIN|将`char16_t*`转换成`wchar_t*`|
|[cr::AsUnicode32]()|POSXI|将`wchar_t*`转换成`char32_t*`|
|[cr::AsConstUnicode32]()|POSIX|将`char32_t*`转换成`wchar_t*`|

#### 字符串连接
```c++
/* 头文件 */
#include "cr_base/string/string_cat.h"
```
|函数|描述|
|:--|:--|
|[cr::StrCat]()|依次连接字符串组的所有内容|
|[cr::StrAppend]()|依次连接字符串组的所有内容，并且追加到指定的目标|
|[cr::JoinString]()|将字符串组的所有内容使用分隔符依次连接|

#### 字符串分割
```c++
/* 头文件 */
#include "cr_base/string/string_split.h"
```
|函数|描述|
|:--|:--|
|[cr::SplitString]()|使用字符表分割字符串，返回`std::vector<std::basic_string>`|
|[cr::SplitStringPiece]()|使用字符表分割字符串视图，返回`std::vector<cr::BasicStringPiece>`，减少内存Allocate|
|[cr::SplitStringIntoKeyValuePairs]()|使用字符分割字符串，并根据键值的分隔符分离出键与值|
|[cr::SplitStringIntoKeyValuePairsUsingSubstr]()|使用字符串分割字符串，并根据键值的分隔符分离出键与值|
|[cr::SplitStringUsingSubstr]()|使用字符串分割字符串，返回`std::vector<std::basic_string>`|
|[cr::SplitStringPieceUsingSubstr]()|使用字符串分割字符串视图，返回`std::vector<std::BasicStringPiece>`，减少内存Allocate|