# 内存

#### 智能指针类

###### [cr::RefPtr\<T\>](./memory/ref_ptr.md)
>引用计数对象实例管理类，用于实例生命周期自动管理。
* [头文件]()
```c++
#include "cr_base/memory/ref_counted.h"
```

* 样例
```c++
// 非线程安全对象
class Foo : public cr::RefCounted<Foo> {
 public:
  Foo() = default;
};

// 线程安全对象
class ThreadSafeFoo : public cr::ThreadSafeRefCounted<Foo> {
 public:
  ThreadSafeFoo() = default;
};

// 创建对象
cr::RefPtr<Foo> foo1 = cr::MakeRefCounted<Foo>();
cr::RefPtr<ThreadSafeFoo> foo2 = cr::MakeRefCounted<ThreadSafeFoo>();
```

###### [cr::WeakPtr\<T\>](./)
>弱指针类，不会销毁管理的实例，能够获取其销毁情况，用于在异步方法以及两个互相引用计数管理的实例中安全访实例。  
**NOTE**:  
该类内部有一个带有引用计数管理的销毁标识符对象, 在实例销毁后该逻辑标识符会被设置为已销毁状态。

* [头文件]()
```c++
#include "cr_base/memory/weak_ptr.h                         "
```

___
#### 单例类

###### [cr::NoDestructor\<T\>](./memory/no_destructor.md)
>辅助生产无析构单例类，用于在栈空间上创建对象单例，其析构函数不会被调用, 适用于不动态分配内存的单例。
* [头文件]()
```c++
#include "cr_base/memory/no_destructor.h"
```

###### [cr::Singleton\<T\>]()
>辅助生产单例类，在程序退出过程中通过[cr::AtExitManager]()自动释放单例。
* [头文件]()
```c++
#include "cr_base/memory/singleton.h"
```
