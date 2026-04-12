#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if(argc <= 2)
    {
        printf("usage: %s ip_address port_number\n", basename(argv[0]));
        return 1;
    }
    const char *ip = argv[1];
    int port = atoi(argv[2]);

    sockaddr_in address;  // IPV4地址结构
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr); // IPV4点分十进制转换成整型，网络字节序
    address.sin_port = htons(port);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    assert(sock >= 0);

    int ret = bind(sock, (sockaddr *)&address, sizeof(address));
    assert(ret != -1);

    ret = listen(sock, 5);
    assert(ret != -1);

    sockaddr_in client_address;
    socklen_t client_addrlength = sizeof(client_address);
    int connfd = accept(sock, (sockaddr*)&client_address, &client_addrlength);
    if(connfd < 0)
    {
        printf("errno is: %d\n", errno);
    }
    else
    {
        close(STDOUT_FILENO); // 关掉绑定到标准输出的文件描述符
        dup(connfd); // 生成新的文件描述符，绑定到connfd对应的socket上
        printf("abcd\n");  // 往文件描述符1中写数据，现在文件描述符1绑定到connfd对应的socket连接上，相当于直接写到socket连接，而不是输出在终端
        close(connfd); // 关掉最开始绑定到socket连接的文件描述符，此时socket连接不会立即关闭，因为还有一个fd指向socket连接，引用计数不等于零
    }

    close(sock);
    return 0;
}