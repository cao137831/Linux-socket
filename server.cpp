#include <iostream>
#include <string.h>

#include <unistd.h>
#include <arpa/inet.h>  // 包含头文件 #include <sys/socket.h>

#include <thread>

#include "./pool/TaskQueue.h"
#include "./pool/ThreadPool.h"
#include "./socket/socket.h"

#define PORT 8888   // 端口
#define LISTEN_NUM 128  // 监听数量

void work(void *arg)
{
    std::cout << " - 通信线程启动\n";
    Socket *accept = static_cast<Socket*>(arg);
    // 通信
    while(1)
    {
        // 接收数据
        char *buff = new char[1024];
        int len = accept->recvMsg(&buff);
        std::cout <<"接收到的数据长度为: "<< len <<'\n';
        if (len > 0){
            // 接收成功
            std::cout << buff <<'\n';
            accept->sendMsg(buff, len);
        }else if (len == 0){
            // 客户端已经断开连接
            std::cout <<"客户端断开连接~\n";
            break;
        }else if (len == -1){
            // 接收数据失败
            std::cout <<"接收数据失败\n";
            break;
        }
        delete []buff;
    }
    std::cout <<" - 通信线程关闭\n";
    delete accept;
}

void acceptConn(Socket *socket, ThreadPool *pool)
{
    std::cout << " - 开始监听\n";
    // 阻塞并等待客户端的连接
    while (1)
    {
        struct sockaddr_in client_addr;
        Socket *accept = socket->setAccept(&client_addr);

        // 与客户端建立连接成功，输出信息
        char ip[32];
        printf("----成功建立连接----\n客户端IP: %s\n端口: %d\n", 
                inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, ip, sizeof(ip)),
                ntohs(client_addr.sin_port));

        // 创建子线程 并将通信任务加入线程池
        pool->addTask(Task(work, accept));
    }

    std::cout <<" - 关闭监听\n";
}

int main()
{
    // 创建监听套接字
    Socket socket(AF_INET, SOCK_STREAM, 0);

    // 绑定本地的IP PORT
    struct sockaddr_in localaddr;
    localaddr.sin_family = AF_INET;
    localaddr.sin_addr.s_addr = INADDR_ANY;
    localaddr.sin_port = htons(PORT);

    socket.setBind(&localaddr);

    // 设置监听
    socket.setListen(LISTEN_NUM);

    // 创建线程池用于连接客户端
    ThreadPool pool(2, 3);   // 客户端的最小连接数是2, 最大连接数是3

    // 创建接受客户端连接的线程
    std::thread acceptThread(acceptConn, &socket, &pool);
    acceptThread.join();

    return 0;
}