#include "socket.h"

int Socket::get_fd()
{
    return this->fd;
}

Socket::Socket(int __domain, int __type, int __protocol)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    this->fd = fd;
    if (fd == -1){
        std::cout <<"套接字创建失败\n";
    }
}

Socket::Socket(int fd)
{
    this->fd = fd;
}

Socket::~Socket()
{
    close(fd);
}

void Socket::setBind(const sockaddr_in *addr)
{
    int ret = bind(fd, (struct sockaddr*)addr, sizeof(addr));
    if (ret == -1){
        std::cout << "绑定套接字失败\n";
        exit(-1);
    }
}

void Socket::setListen(int n)
{
    int ret = listen(fd, n);
    if (ret == -1){
        std::cout << "设置监听失败\n";
        exit(-1);
    }
}

Socket *Socket::setAccept(const sockaddr_in *addr)
{
    socklen_t addrLen = 0;
    int accept_fd = accept(fd, (struct sockaddr*)addr, &addrLen);
    if (accept_fd == -1){
        std::cout << "接收客户端连接失败";
    }
    return new Socket(accept_fd);
}

int Socket::readn(char *msg, int size)
{
    char *buff = msg;
    int count = size;
    while (count > 0)
    {
        int len = recv(fd, buff, count, 0);
        if (len == -1){
            return -1;
        }else if (len == 0){    // 说明客户端已经断开连接
            return size - count;
        }
        buff += len;
        count -= len;
    }
    return size;
}

int Socket::recvMsg(char** msg)
{
    // 先接收 int 类型的长度
    int len = 0;
    readn((char*)&len, sizeof(int));
    len = ntohl(len);
    std::cout <<"接收的数据块长度为: "<< len <<'\n';

    char *data = (char*)malloc(len + 1);    // +1是\0的意思
    int ret = readn(data, len);
    if (ret != len){
        std::cout <<"接收数据失败了。。。\n";
        close(fd);
        free(data);
        data = NULL;
        return -1;
    }
    data[ret] = '\0';
    *msg = data;
    return ret;
}

void Socket::setConnect(const sockaddr_in *addr)
{
    int ret = connect(fd, (struct sockaddr*)addr, sizeof(addr));
    if (ret == -1){
        std::cout <<"连接服务器失败\n";
        exit(-1);
    }
    std::cout << "与服务器建立连接\n";
}

int Socket::writen(const char *msg, int size)
{
    const char *buff = msg;
    int count = size;
    while (count > 0)
    {
        int len = send(fd, buff, count, 0);
        if (len == -1){
            return -1;
        }else if (len == 0){
            continue;
        }
        buff += len;
        count -= len;
    }
    return size;
}

int Socket::sendMsg(const char *msg, int len)
{
    if (fd < 0 || msg == NULL || len <= 0){
        return -1;
    }
    char *data = (char*)malloc(len + 4);
    int bigLen = htonl(len);
    memcpy(data, &bigLen, sizeof(int));
    memcpy(data + sizeof(int), msg, len);

    int ret = writen(data, len + sizeof(int));
    if (ret == -1){
        return -1;
        close(fd);
    }
    return ret;
}