

//通过c程序调试strace的命令使用
//2.调试进程跟踪  strace -f debug_starce_03
#include <stdio.h>
#include <unistd.h>


int main()
{
    pid_t fpid; //fpid表示fork函数返回的值
    fpid = fork();//for创建子进程
    if (fpid < 0)
        printf("error in fork!");//创建子进程失败
    else if (fpid == 0)//子进程
    {
        printf("i am the child process, my process id is %d/n", getpid());
         //做一个简单的写文件和读文件操作
        FILE *pf = NULL;
        char buf[512] = {0};


        pf = fopen("./test.txt", "a+");//打开一个文件，文件不存在则创建
        if(NULL == pf)
        {
            printf("open file err\n");
            return -1;
        }

        //文件追加一行
        fputs("test\n",pf);
        //读取文件
        while(fgets(buf,512,pf) != NULL)
        {
            printf("%s",buf);
        }
        printf("\n");

        fclose(pf);//关闭文件
    }
    else//父进程
    {
        printf("i am the parent process, my process id is %d/n", getpid());
    }

   
    return 0;
}