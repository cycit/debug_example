

//通过c程序调试strace的命令使用

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>


int main()
{

    pid_t pid = getpid();
    kill(pid,2);


    return 0;
}