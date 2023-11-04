#pragma once
#include <sys/socket.h>
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>

class Socket
{
private:
    int fd;
public:
    int get_fd();
    // 创建socket
    Socket(int __domain, int __type, int __protocol);
    Socket(int fd);
    // 释放socket
    ~Socket();
    // 绑定操作
    void setBind(const sockaddr_in *addr);
    // 设置监听
    void setListen(int n);
    // 等待客户端的连接
    Socket *setAccept(const sockaddr_in *addr);
    // 将数据写入读缓冲区中，用于接收数据
    int readn(char *msg, int size);
    // 发送数据
    int recvMsg(char** msg);

    // 连接操作
    void setConnect(const sockaddr_in *addr);
    // 将数据写入写缓冲区中，用于发送数据
    int writen(const char *msg, int size);
    // 发送数据
    int sendMsg(const char *msg, int len);
};