# 内存

### 智能指针类

###### [cr::RefPtr\<T\>](./memory/ref_ptr.md)
>引用计数对象实例管理类，用于实例生命周期自动管理。
```c++
/* 头文件 */
#include "cr_base/memory/ref_counted.h"
```

###### [cr::WeakPtr\<T\>](./)
>弱指针类，能够获取实例的销毁情况，用于在异步方法中安全访实例。  
该类内部有一个带有引用计数管理的销毁标识符对象, 在实例销毁后该逻辑标识符会被设置为已销毁状态。

```c++
/* 头文件 */
#include "cr_base/memory/weak_ptr.h                         "
```

___
### 单例类

###### [cr::NoDestructor\<T\>](./memory/no_destructor.md)
>辅助生产无析构单例类，用于在栈空间上创建对象单例，其析构函数不会被调用, 适用于不动态分配内存的单例。
```c++
/* 头文件 */
#include "cr_base/memory/no_destructor.h"
```

###### [cr::Singleton\<T\>]()
>辅助生产单例类，在程序退出过程中通过[cr::AtExitManager]()自动释放单例。
```c++
/* 头文件 */
#include "cr_base/memory/singleton.h"
```
