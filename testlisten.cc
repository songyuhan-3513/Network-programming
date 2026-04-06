#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

static bool stop = false;  // C语言原身不支持bool类型

static void handle_term(int sig) {
    stop = true;
}

int main(int argc, char *argv[]) {
    signal(SIGTERM, handle_term);

    if(argc <= 3)
    {
        printf("usage: %s ip_address port_number bakclog\n", basename(argv[0]));
        return 1;
    }

    const char *ip = argv[1];
    int port = atoi(argv[2]);  // atoi 将C风格字符串转换为整数
    int backlog = atoi(argv[3]);

    int sock = socket(PF_INET, SOCK_STREAM, 0);
    assert(sock >= 0);

    struct sockaddr_in address;  // IPV4地址结构
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr); // IPV4点分十进制转换成整型，网络字节序
    address.sin_port = htons(port);            // 端口号转换成网络字节序

    int ret = bind(sock, (struct sockaddr*)&address, sizeof(address));
    assert(ret != -1);

    ret = listen(sock, backlog);
    assert(ret != -1);

    while(!stop)
    {
        sleep(1);
    }

    close(sock);
    return 0;
}




