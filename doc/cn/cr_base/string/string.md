## 字符串操作 (非国际化)
* 头文件："[cr_base/strings/string_util.h]()"

#### 大小写转换

|函数|描述|
|:--|:--|
|[cr::ToLowerASCII]()|ASCII字符转小写|
|[cr::ToUpperASCII]()|ASCII字符转大写|

#### 字符串比较

|函数|描述|
|:--|:--|
|[cr::CompareCaseInsensitiveASCII]()|比较两个字符串大小，区分ASCII字符大小写|
|[cr::EqualsCaseInsensitiveASCII]()|比较两个字符串是否一致，区分ASCII字符大小写|

#### 空字符单例

|函数|描述|
|:--|:--|
|[cr::EmptyString](/#)|返回一个空的`std::string`单例对象|
|[cr::EmptyString16](/#)|返回一个空的`std::u16string`单例对象|
|[cr::EmptyString32](/#)|返回一个空的`std::u32string`单例对象|
|[cr::EmptyWString](/#)|返回一个空的`std::wstring`单例对象|

#### 字符串 移除/替换/填充

|函数|描述|
|:--|:--|
|[cr::RemoveChars]()|移除指定的字串|
|[cr::ReplaceChars]()|替换指定字串|
|[cr::TrimString]()|根据字符表，裁剪字符串首尾|
|[cr::TrimWhitespaceASCII]()|裁剪字符串的首尾空ASCII白字符|
|[cr::TrimWhitespace]()|裁剪字符串首尾空白字符|
|[cr::ReplaceFirstSubstringAfterOffset]()|替换首个子串|
|[cr::ReplaceSubstringsAfterOffset]()|替换所有子串|
|[cr::ReplaceStringPlaceholders]()|替换字符`'$'`|
|[cr::WriteInto]()|以指定的尺寸，重置`std::basic_string`缓冲区。|

#### 字符串判断

|函数|描述|
|:--|:--|
|[cr::ContainsOnlyChars]()|字符串是否只含有给定字符表里的字符|
|[cr::IsStringUTF8]()|字符串是否为UTF-8编码，不允许有非字符出现(66个特定的代码点)|
|[cr::IsStringUTF8AllowingNoncharacters]()|字符串是否为UTF-8编码，允许非字符(66个特定的代码点)|
|[cr::IsStringASCII]()|字符串是否全为ASCII字符|
|[cr::LowerCaseEqualsASCII]()|字符串转换成ACSII小写格式后验证是否与指定的纯ASCII字符串（小写）一致|
|[cr::EqualsASCII]()|字符串\|宽字符串与指定的字符串是否一致|
|[cr::StartsWith]()|给定的字符串是否以指定的字符串开头，支持区分ASCII大小写|
|[cr::EndsWith]()|给定的字符串是否以指定的字符结尾，支持区分ASCII大小写|
|[cr::IsAsciiWhitespace]()|字符是否为ACSII空白字符|
|[cr::IsAsciiAlpha]()|字符是否为英文'a'-'z'或'A'-'Z'|
|[cr::IsAsciiUpper]()|字符是否为大写英文'A-'Z'|
|[cr::IsAsciiLower]()|字符是否为小写英文'a'-'z'|
|[cr::IsAsciiDigit]()|字符是否为数字'0'-'9'|
|[cr::IsAsciiPrintable]()|字符是否可以在控制台打印|
|[cr::IsHexDigit]()|字符是否为Hex数字|

#### C-Style标准操作

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
在其他POSIX平台可能就会占用4个字节，具体见宏`MINI_CHROMIUM_WCHAR_IS_UTF16`。

|函数|平台|描述|
|:--|:--|:--|
|[cr::AsWide]()||WIN：将`char16_t*`转换成`wchar_t*`</br>POSIX：将`char32_t*`转换成`wchar_t*`|
|[cr::AsConstWide]()||WIN：将`const char16_t*`转换成`const wchar_t*`</br>POSIX：将`const char32_t*`转换成`const wchar_t*`|
|[cr::AsUnicode16]()|WIN|将`wchar_t*`转换成`char16_t*`|
|[cr::AsConstUnicode16]()|WIN|将`char16_t*`转换成`wchar_t*`|
|[cr::AsUnicode32]()|POSXI|将`wchar_t*`转换成`char32_t*`|
|[cr::AsConstUnicode32]()|POSIX|将`char32_t*`转换成`wchar_t*`|

#### 字符串连接
    
* 头文件: ["cr_base/strings/strcat.h"]()

|函数|描述|
|:--|:--|
|[cr::StrCat]()|依次连接字符串组的所有内容|
|[cr::StrAppend]()|依次连接字符串组的所有内容，并且追加到指定的目标|
|[cr::JoinString]()|将字符串组的所有内容使用分隔符依次连接|


#### 字符串分割
    
* 头文件: ["cr_base/strings/strcat.h"]()

|函数|描述|
|:--|:--|
|[cr::SplitString]()|使用字符表分割字符串，返回`std::vector<std::basic_string>`|
|[cr::SplitStringPiece]()|使用字符表分割字符串，返回`std::vector<cr::BasicStringPiece>`，减少内存Allocate|
|[cr::SplitStringIntoKeyValuePairs]()|使用字符分割字符串，并根据键值的分隔符分离出键与值|
|[cr::SplitStringIntoKeyValuePairsUsingSubstr]()|使用字符串分割字符串，并根据键值的分隔符分离出键与值|
|[cr::SplitStringUsingSubstr]()|使用字符串分割字符串，返回`std::vector<std::basic_string>`|
|[cr::SplitStringPieceUsingSubstr]()|使用字符串分割字符串，返回`std::vector<std::BasicStringPiece>`，减少内存Allocate|

#### 字符串&数字互转
    
* 头文件: ["cr_base/strings/string_number_conversion.h"]()

|函数|描述|
|:--|:--|
|[cr::NumberToString]()|数字转成字符串`std::string`|
|[cr::NumberToString16]()|数字转成字符串`std::u16string`|
|[cr::NumberToString32]()|数字转成字符串`std::u32string`|
|[cr::NumberToWString]()|数字转成字符串`std::wstring`|
|||
|[cr::StringToInt]()|数字字符串转`int`|
|[cr::StringToUint]()|数字字符串转`unsigned int`|
|[cr::StringToInt64]()|数字字符串转`int64_t`|
|[cr::StringToUint64]()|数字字符串转`unsigned int64_t`|
|[cr::StringToSizeT]()|数字字符串转`size_t`|
|[cr::StringToDouble]()|数字字符串转`double`|
|||
|[cr::HexEncode]()|将字节组转成HEX字符串|
|[cr::HexStringToInt]()|将HEX字符串(-0x80000000 - 0x7FFFFFFF)转成`int`|
|[cr::HexStringToUInt]()|将HEX字符串(-0x80000000 - 0x7FFFFFFF)转成`unsigned int`|
|[cr::HexStringToInt64]()|将HEX字符串(-0x8000000000000000 - 0x7FFFFFFFFFFFFFFF)转成`int64_t`|
|[cr::HexStringToUInt64]()|将HEX字符串(-0x8000000000000000 - 0x7FFFFFFFFFFFFFFF)转成`unsigned int64_t`|
|[cr::HexStringToBytes]()|将HEX字符串转成字节组，储存到`std::vector<uint8_t>`|
|[cr::HexStringToString]()|将HEX字符串转成字节组，储存到`std::string`|
|[cr::HexStringToSpan]()|将HEX字符串转成字节组，储存到已经固定尺寸的字节组缓冲区|

