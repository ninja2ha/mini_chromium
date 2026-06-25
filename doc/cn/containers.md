# 标准容器
#### 类
###### [cr::circular_deque]()
```c++
// 头文件 
// #include "cr_base/conatiners/circular_deque.h"
```
>环形双端队列类，避免普通双端队列的假溢出导致内存空间利用不足问题。

###### [cr::queue]()
>队列容器类，使用环形双端队列实现的std::queue。
```c++
// 头文件 
// #include "cr_base/conatiners/queue.h"
```

###### [cr::stack]()
>栈容器类，使用环形双端列实现的std::stack。
```c++
// 头文件 
// #include "cr_base/conatiners/stack.h"
```

###### [cr::optional]()
>可选值类，用于解决值可能存在也可能不存在的问题。
```c++
// 头文件 
// #include "cr_base/conatiners/optional.h"
```

###### [cr::Span]()
>连续内存访问视图类，用于快速访问连续内存。
``` c++
// 头文件
#include "cr_base/conatiners/span.h"
```

#### 相关函数
``` c++
// 头文件
#include "cr_base/stl_util.h"
```
|函数|描述|
|:---|:---|
|[cr::size]()|获取数组、容器的元素个数|
|[cr::empty]()|检查数组、容器是否含有元素|
|[cr::to_address]()|获取数组、容器迭代器数据地址|
|[cr::data]()|获取数组、容器数据起始地址|
|[cr::contains]()|根据元素值，检查数组、容器中是否含有指定元素|
|[cr::erase]()|根据元素值， 移除容器里的指定元素|
|[cr::erase_if]()|按条件移除容器元素|

