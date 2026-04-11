#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define BUFFER_SIZE 1023

/* 将文件描述符设置为非阻塞 */
int setnonblocking(int fd) {
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

int unblock_connect(const char *ip, int port, int time) {
    int ret = 0;
    sockaddr_in address;  // IPV4地址结构
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr); // IPV4点分十进制转换成整型，网络字节序
    address.sin_port = htons(port);

    int sockfd = socket(PF_INET, SOCK_STREAM, 0);
    int fdopt = setnonblocking(sockfd);
    ret = connect(sockfd, (sockaddr*)&address, sizeof(address));
    if(ret == 0)
    {
        printf("connect with server immediately\n");
        fcntl(sockfd, F_SETFL, fdopt);  // 将连接socket设置回阻塞状态
        return sockfd;
    }
    else if(errno != EINPROGRESS) // 对非阻塞socket调用connect，连接没有立即建立
    {
        printf("unblock connect not support\n");
        return -1;
    }

    fd_set readfds;
    fd_set writefds;
    timeval timeout;

    FD_ZERO(&readfds);
    FD_SET(sockfd, &writefds); // 设置监听sockfd上的读事件

    timeout.tv_sec = time;
    timeout.tv_usec = 0;

    /* 对于非阻塞 socket，当 connect() 后连接建立成功（或失败），该 socket 会被标记为可写。因此，通过检查可写性可以判断连接是否完成。 */
    ret = select(sockfd + 1, NULL, &writefds, NULL, &timeout);
    if(ret <= 0)
    {
        printf("connection time out\n");
        close(sockfd);
        return -1;
    }

    if(!FD_ISSET(sockfd, &writefds))
    {
        printf("no events on sockfd found\n");
        close(sockfd);
        return -1;
    }

    int error = 0;
    socklen_t length = sizeof(error);
    if(getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &length) < 0)
    {
        printf("get socket option failed\n");
        close(sockfd);
        return -1;
    }

    if(error != 0)
    {
        printf("connection failed after select with the error: %d \n", error);
        close(sockfd);
        return -1;
    }

    printf("connection ready after select with the socket: %d \n", sockfd);
    fcntl(sockfd, F_SETFL, fdopt);
    return sockfd;
}

int main(int argc, char *argv[]) {
    if(argc <= 2)
    {
        printf("usage: %s ip_address port_number\n", basename(argv[0]));
        return 1;
    }

    const char *ip = argv[1];
    int port = atoi(argv[2]);

    int sockfd = unblock_connect(ip, port, 10);
    if(sockfd < 0)
    {
        return 1;
    }
    close(sockfd);
    return 0;
}
