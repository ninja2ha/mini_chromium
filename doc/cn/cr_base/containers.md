# 容器
#### 类
###### [cr::circular_deque]()
>头文件: `"cr_base/conatiners/circular_deque.h"`</br>环形双端队列类。避免普通双端队列的假溢出导致内存空间利用不足问题。

###### [cr::queue]()
>头文件: `"cr_base/conatiners/queue.h"`</br>队列容器类，使用环形双端队列实现的std::queue。

###### [cr::stack]()
>头文件: `"cr_base/conatiners/stack.h"`</br>栈容器类，使用环形双端列实现的std::stack。

###### [cr::optional]()
>头文件: `"cr_base/conatiners/optional.h"`</br>可选值类。

###### [cr::span]()
>头文件: `"cr_base/conatiners/span.h"`</br>数组视图类。


#### 函数
``` c++
// 头文件
#include "cr_base/conatiners/conatiner_util.h"
```
|||
|:---|:---|
|[cr::size]()|获取数组、容器元素个数。|
|[cr::empty]()|检查数组、容器是否含有元素。|
|[cr::to_address]()|获取数组、容器迭代器数据地址。|
|[cr::data]()|获取数组、容器数据起始地址。|
|[cr::contains]()|根据元素值，检查数组、容器中是否含有指定元素。|
|[cr::erase]()|根据元素值， 移除容器里的指定元素。|
|[cr::erase_if]()|按条件移除容器元素。|

