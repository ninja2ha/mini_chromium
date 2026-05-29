# 内存

#### 类 - 指针
###### [cr::RefPtr\<T\>](./memory/ref_ptr.md)
```c++
// 头文件
#include "cr_base/memory/ref_counted.h"
```
>引用计数对象实例管理类。

###### [cr::WeakPtr\<T\>](./)
```c++
// 头文件
#include "cr_base/memory/weak_ptr.h"
```
>弱指针类，能够获取实例的销毁情况，用于在异步方法中安全访实例，。</br>该类内部有一个带有引用计数管理的销毁标识符对象, 在实例销毁后该逻辑标识符会被设置为已销毁状态。

###### [cr::NoDestructor\<T\>](./memory/no_destructor.md)
```c++
// 头文件
#include "cr_base/memory/no_destructor.h"
```
>生产无析构单例类，用于在.data空间上创建对象单例，其析构函数不会被调用, 适用于不分配内存的单例。