

//通过c程序调试strace的命令使用
//1.strace -c 
//调试输出统计信息：系统调用名称  次数  出错次数  耗时  耗时占比
#include <stdio.h>


int main()
{
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
    return 0;
}