# YRPC



[TOC]



### 一、简介

​		YRPC 是一个基于 boost.Context 的RPC框架。利用boost 协程库的协程上下文切换做跨平台（其实是自己没水平也比较懒啦），自实现有栈非对称协程，封装协程调度器。YRPC最大的卖点就是连接复用、抽象信道。



### 二、项目介绍





##### YRPC结构

###### 	1.YRoutine 模块

​	这一个模块是协程模块，主要就是封装了控制协程栈切换的Scheduler和封装了linux socket和epoll的Epoller。Epoller是构建YRPC协程级IO的关键一步。YRPC仍然是一个基于IO多路复用的Reactor网络模型。在整体逻辑上是这样。

​	协程模型是有栈协程，且通过主协程进行loop，调度子协程。我个人倾向于理解为函数调用。只不过可以随时挂起，虽然本质上也是如此，只不过函数调用只能从 call 指令开始，到ret结束。但是协程随时可以修改栈顶栈底指针、寄存器值（类比线程上下文切换）。本质上做的工作和线程上下文类似。



###### 	2.network 模块

​	虽然Epoller提供了基于IO、超时事件的上下文切换，但是距离一个完整的网络层仍然有距离，所以构建network模块来实现完整的网络通信，这里有点意思的就是Connector采取了ECS的设计理念（就是数据和操作进行分离）

​	也是因为采用ECS，所以和传统的network模块有点不同。这里主要封装了系统网络资源实际持有者——Connection，服务端对新连接监听的抽象——Acceptor，YRPC协议Buffer——SessionBuffer，客户端连接服务端的抽象——Connector。



###### 	3.msg 模块（待整合）

​	msg模块是作为服务提供者需要有服务（Service）结构，那这个结构包括相关功能都在msg下面。提供了服务注册的一个全局Map。



###### 	4.protocol 模块

​	封装protobuf接口，并且定义了YRPC的自定义协议，YRPC协议包含自定义协议头（由我定义），protobuf数据部分（用户发送请求包）。RPC功能由我的自定义协议头实现，但是参数由protobuf去做处理。



###### 	5.util、shared 

​	错误码、定时器、线程池、bytearray、线程安全容器、日志、uid生成器、linux 线程同步RAII风格的封装



###### 	6.core 模块

​	就是yrpc目录。这个是最重要的部分，这里是整个RPC最核心的部分。里面几个比较重要的模块和功能介绍一下

> - Channel：信道的封装，是对一条TCP连接的封装，这里做了协议通路复用。如果正常我们想要连接对端，进行服务调用怎么做？就是构建一个RpcClient，然后连接对端的一个socket，但是通过Channel，我们把一条TCP连接进行封装，本身就是双工通信，那么就不需要两条TCP连接进行数据的收发。
> - RpcSession：一条Session相当于一条连接，和channel不同，它包含channel，且在其之上提供了连接控制和数据包拆解、协议控制等功能。
> - SessionManager：核心中的核心，对上层向RpcClient、RpcServer提供连接管理和控制，如果一条TCP通道将要被创建，那么就可以通过SessionManager接口进行Hook，以此区别是否真的需要建立连接，但是对上层来说就是已经建立了一条连接。主要是处理一个进程同时作为服务提供方和服务调用方导致对系统的网络资源占用问题。
> - RpcServer：提供服务注册。
> - RpcClient：提供服务调用





### 三、测试

​	目前正在重构，请看doc下面之前版本的测试。

    



### 四、项目构建

目前依赖于bbt、boost1.7x、protobuf3.10x

google protobuf 项目地址:

[protocolbuffers/protobuf: Protocol Buffers - Google's data interchange format (github.com)](https://github.com/protocolbuffers/protobuf)

boost 项目地址:

[Boost C++ Libraries](https://www.boost.org/)

bbtools 项目地址:

[yqm-307/biangbiangtools: linux下常用工具库 (github.com)](https://github.com/yqm-307/biangbiangtools)



安装好依赖之后，执行下列命令：

> git clone https://github.com/yqm-307/yrpc.git
>
> cd yrpc/shell
>
> ./build.sh

