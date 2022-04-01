# voiceserver
	基于udp的音频服务器


## 简介
        Reactor模式作为底层网络，尽量抽象出udp网络库。


## 下层

### Reactor
        事件反应器，控制epoller、channels，重要作用就是开一个线程，循环的调用Reactor的poll，poll不断轮询
	epoll_wait，等待事件发生，因为事件被封装为Channel，就调用Channel中的回调通知调用者（OnConnection、
	Close、RecvDate等）

### Acceptor
        封装一个listen套接字，添加进epoll中管理。有新连接，调用OnConnect回调通知调用者。

### Epoller
        本质就是封装一些epoll操作，服务器中所有套接字都在这个epoll中管理，包括套接字的读写、连接的建立等。
    主要监听网络事件。采用LT模式，简化操作。ET模式数据处理太麻烦，容易错误。 

## 上层

### UdpSession
        管理一个单向的udp连接，就是打开一个udp套接字。因为是音视频流，所以不设置缓冲区（其实是懒，后期再加）
    Session包含Coder、OnConnect、OnRecvData、send等操作。

### UdpConnection
		一个单向的udp连接就是一个connection。将socket、ipaddress等封装在UdpConnection中，将sendto、recv
	from等操作封装。主要是封装Kcp操作。
