## 字节序
头文件: cr_base/byte_order.h

### Global Functions

##### Swap
|method|desc|
|:---|:---|
|cr::ByteSwap|反转字节序。||
|cr::ByteSwapUintPtrT|反转字节序。||
|cr::ByteSwapToLE16|将16位整数从主机字节序交换到小端字节序。||
|cr::ByteSwapToLE32|将32位整数从主机字节序交换到小端字节序。||
|cr::ByteSwapToLE64|将64位整数从主机字节序交换到小端字节序。||

##### NetToHost
|method|desc|
|:---|:---|
|cr::NetToHost16|将16整数从网络(大端)字节序转成主机字节序。||
|cr::NetToHost32|将32整数从网络(大端)字节序转成主机字节序。||
|cr::NetToHost64|将64整数从网络(大端)字节序转成主机字节序。||


##### HostToNet
|method|desc|
|:---|:---|
|cr::HostToNet16|将16整数从主机字节序转成网络(大端)字节序。||
|cr::HostToNet32|将32整数从主机字节序转成网络(大端)字节序。||
|cr::HostToNet64|将64整数从主机字节序转成网络(大端)字节序。||

##### Memory operations(BE)
|method|desc|
|:---|:---|
|cr::SetBE16|将16位整数从主机字节序转换到大端字节序并写入内存。||
|cr::SetBE32|将32位整数从主机字节序转换到大端字节序并写入内存。||
|cr::SetBE64|将64位整数从主机字节序转换到大端字节序并写入内存。||
|cr::GetBE16|从内存读取16位整数(大端字节序)并转换到主机字节序。||
|cr::GetBE32|从内存读取32位整数(大端字节序)并转换到主机字节序。||
|cr::GetBE64|从内存读取64位整数(大端字节序)并转换到主机字节序。||

##### Memory operations(LE)
|method|desc|
|:---|:---|
|cr::SetLE16|将16位整数从主机字节序转换到小端字节序并写入内存。|
|cr::SetLE32|将32位整数从主机字节序转换到小端字节序并写入内存。|
|cr::SetLE64|将64位整数从主机字节序转换到小端字节序并写入内存。|
|cr::GetLE16|从内存读取16位整数(小端字节序)并转换到主机字节序。|
|cr::GetLE32|从内存读取32位整数(小端字节序)并转换到主机字节序。|
|cr::GetLE64|从内存读取64位整数(小端字节序)并转换到主机字节序。|