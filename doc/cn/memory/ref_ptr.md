# 内存 - 引用计数对象

#### 类
###### [cr::RefPtr\<T\>]()
```c++
// 头文件
#include "cr_base/memory/ref_counted.h"
```
引用计数对象实例管理类。

* 模板参数

|||
|:--|:--|
|T|引用计数对象, 需要继承自`"非线程安全的"cr::RefCounted<T>` 或</br>`"线程安全的"cr::RefCountedThreadSafe<T>`|

* 成员函数

|||
|:--|:--|
|[constructor]()|构造函数|
|[deconstructor]()|析构函数|
|||
|[operator*]()|访问指针指向的对象|
|[operator->]()|访问管理的指针|
|[operator=]()|赋值一个新指针|
|[operator bool]()|检查管理的指针是否为空|
|[operator==]()|比较当前指针与给定的指针是否一致|
|[operator!=]()|比较当前指针与给定的指针是否不一致|
|[operator<]()|比较当前指针是否小于给定的指针|
|||
|[reset]()|重置管理的指针|
|[release]()|释放一次当前管理的指针|
|[swap]()|交换对象|

* 非成员函数

|||
|:--|:--|
|[operator==]()|比较两个指针是否一致|
|[operator!=]()|比较两个指针是否不一致|

* 辅助函数

|||
|:--|:--|
|[cr::MakeRefCounted]()|生产一个引用计数对象实例并封装管理|
|[cr::WrapRefCounted]()|封装管理已有的引用计数对象实例|

___
###### [cr::RefCounted\<T\>]()
>引用计数对象基类(非线程安全)

* 例子
```c++
#include "cr_base/memory/ref_counted.h"

class Foo : public cr::RefCounted<Foo> {
 public:
  Foo() = default;
  ~Foo() = default;
}

int main() {
  cr::RefPtr<Foo> foo = cr::MakeRefCounted<Foo>();
  return 0;  
}

```
___
###### [cr::RefCountedThreadSafe\<T\>]()
>引用计数对象基类(线程安全)

* 例子
```c++
#include "cr_base/memory/ref_counted.h"

class Foo : public cr::RefCountedThreadSafe<Foo> {
 public:
  Foo() = default;
  ~Foo() = default;
}

int main() {
  cr::RefPtr<Foo> foo = cr::MakeRefCounted<Foo>();
  return 0;  
}

```
