#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

bool daemonize()
{
    pid_t pid = fork();
    if(pid < 0)
    {
        return false;
    }
    else if(pid > 0)
    {
        exit(0); // 关掉父进程，变成孤儿进程
    }
    umask(0);
    pid_t sid = setsid(); // 创建新的会话
    if(sid < 0)
    {
        return false;
    }
    if(chdir("/") < 0)
    {
        return false;
    }

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    open("dev/null", O_RDONLY); // 将文件描述符1绑定到dev/null
    open("dev/null", O_RDWR);   // 将文件描述符2绑定到dev/null
    open("dev/null", O_RDWR);   // 将文件描述符3绑定到dev/null
    return true;
}