# *cr::* BasicStringPiece

>字符串视图类  
头文件：["cr_base/strings/string_piece.h"]()

``` c++
// defines
template <typename CharT>
class BasicStringPiece;
```

## 模板参数

|||
|:---|:---|
|`CharT`|字符类型|

## 嵌套类型
|类型|定义|
|:---|:---|
|`size_type`|`size_t`|
|`traits_type`|`std::char_traits<CharT>`|
|`value_type`|`CharT`|
|`pointer`|`const CharT*`|
|`reference`|`const CharT&`|
|`const_reference`|`const CharT&`|
|`difference_type`|`std::ptrdiff_t`|
|`const_iterator`|`const CharT*`|
|`const_reverse_iterator`|`std::reverse_iterator<const_iterator>`|

## 常量成员
|类型|成员|值|说明|
|:---|:---|:---|:---|
|`static size_type`|`npos`|`size_type(-1)`|无效位置|

## 数据成员
|类型|成员|说明|
|:---|:---|:---|
|`const CharT*` |`ptr_`|字符串指针|
|`size_type` |`length_`|字符串长度|


## 函数成员
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
|[operator std::basic_string]()|隐式转换`std::basic_string`到`BasicStringPiece`|
|[begin]()|返回指向开头的迭代器|
|[end]()|返回指向结尾的迭代器|
|[rbegin]()|返回指向开头的反向迭代器|
|[rend]()|返回指向结尾的反向迭代器|
|[max_size]()|获取字符串最大长度，实际返回的是数据成员`length_`|
|[capacity]()|获取字符串的容量，实际返回的是数据成员`length_`|
|[copy]()|拷贝当前字符串到给定的地址|
|[substr]()|获取子字符串视图|
|[compare]()|比较两个字符串|
|[find]()|查找字符串|
|[rfind]()|反向查找字符串|
|[find_first_of]()|在给定的字符串中，查找其子字符出现的首个位置|
|[find_last_of]()|在给定的字符串中，查找其子字符出现的最后位置|
|[find_first_not_of]()|在给定的字符串中，查找其子字符缺席的首个位置|
|[find_last_not_of]()|在给定的字符串中，查找其子字符缺席的最后位置|