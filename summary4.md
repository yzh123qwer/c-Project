# 计算机网络
学习了一些计算机网络最基础的概念。
## 应用层
计算机中的逻辑接口，套接字，通常存在0-65535个接口(2^16)其中0-1023紧密绑定一些特定的服务，如80http。<br>
了解一些一些应用层最基础的协议，主要是功能方面，如web使用的超文本传输协议http，电子邮件使用的smtp，文件传输的ftp。<br>
从应用层方面看待网络，只需知道tcp协议为应用层的程序提供了稳定可靠的数据传输服务，udp只能发送出去，但是不关心对方是否接收到。
## 传输层
了解一些tcp协议的基础知识。<br>
在网络层协议中的ip协议是尽力而为的，因此tcp需要在此基础上保证数据的完整和顺序。<br>
网络层实现主机到主机之间的逻辑通信，而传输层在此基础上实现了主机的程序到主机的程序的通信，但是传输层依然是运行在端系统上的，在实现程序之间的通信时，主要规定消息的收发端口。
# Linux中g++编译器常用指令
了解cpp文件是如何一步一步转换为可执行文件。<br>
预处理阶段(.i文件)->编译生成汇编指令(.s文件)->汇编(.o文件)->链接(可执行文件)。<br>
g++编译的一些指令具体含义，如-o,-g等。
# 项目进展
这周完成了基于tcp的文件传输系统。<br>
传输的报文才用一些简单的约定方式，前4个字节使用int类型，规定该条报文长度。同时也解决了tcp传输中粘包和分包的问题。<br>
在tcp连接建立完成之后，由客户端向服务端程序发送报文，包括一些需要服务端上传、下载文件，服务端收到报文并解析之后，回应客户端即可开始文件传输。<br>
服务端程序通过调用fork函数实现1对多的传输。
