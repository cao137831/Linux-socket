#include <iostream>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include "./socket/socket.h"
#define PORT 8888   // 端口

int main()
{
    // 创建客户端套接字
    Socket socket(AF_INET, SOCK_STREAM, 0);

    // 与服务器建立连接
    struct sockaddr_in clientaddr;
    clientaddr.sin_family = AF_INET;
    // 填写客户端实际的IP
    // inet_pton(AF_INET, "10.2.190.145", &clientaddr.sin_addr.s_addr);
    inet_pton(AF_INET, "127.0.0.1", &clientaddr.sin_addr.s_addr);
    clientaddr.sin_port = htons(PORT);
    socket.setConnect(&clientaddr);

    // 连接成功，开始通信
    while (1)
    {
        char *buff = new char[1024];
        std::cin.getline(buff, 1024);
        // 发送数据
        if (strcmp(buff, "quit") == 0){
            std::cout <<"断开连接~\n";
            break;
        }

        // 解决粘包问题
        int ret = socket.sendMsg(buff, strlen(buff) + 1);
        if (ret == -1){
            std::cout <<"数据发送失败\n";
            break;
        }
        // 接收数据
        memset(buff, 0, sizeof(buff));
        int len = socket.recvMsg(&buff);
        if (len > 0){
            // 接收成功
            std::cout << "server: " << buff <<'\n';
        }else if (len == 0){
            // 客户端已经断开连接
            std::cout <<"服务器已经断开连接\n";
            break;
        }else if (len == -1){
            // 接收数据失败
            std::cout <<"接收数据失败\n";
            delete []buff;
            exit(-1);
        }
        delete []buff;
    }
    return 0;
}