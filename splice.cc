#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

int main(int argc, char *argv[]) {
    if(argc <= 2)
    {
        printf("usage: %s ip_address port_number filename\n", basename(argv[0]));
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
        int pipefd[2];
        assert(ret != -1);
        ret = pipe(pipefd);
        ret = splice(connfd, NULL, pipefd[1], NULL, 32768, SPLICE_F_MOVE | SPLICE_F_MORE);
        assert(ret != -1);
        ret = splice(pipefd[0], NULL, connfd, NULL, 32768, SPLICE_F_MOVE | SPLICE_F_MORE);
        assert(ret != -1);
        close(connfd);
    }

    close(sock);
    return 0;
}