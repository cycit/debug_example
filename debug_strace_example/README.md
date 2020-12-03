通过实验学会Linux工具strace的使用

## 一、介绍

strace是Linux环境下的一个程序调试工具，集成诊断、调试和统计于一身，常用来跟踪进程执行时发生的系统调用和所接收到的信号。通过strace我们可以知道程序打开了哪些文件，读写了什么内容，消耗了多少时间，返回值是啥灯问题。

strace的底层使用的是内核的ptrace来实现的，在程序调试中，对于用户态的问题，我们可以根据程序源代码进行分析定位，而内核态发生的事情和出现的错误，我们没有源代码的情况下，想要跟踪程序，则是比较困难的事情，而strace这样的工具，可以显示有关进程的系统调用信息，帮助我们去确定程序发生了哪些系统调用，收到了哪些信号，还可以定位系统调用过程中失败的问题。

**补充**

在操作系统，进程空间分为用户空间和内核空间，进程运行分为用户态进程和内核态进程。用户态访问权限和能力是受限的，进程只能访问有限的内存范围，不允许访问外围设备，cpu资源可以被其他程序抢占。内核态下，cpu可以访问内存中所有数据，同时能访问外围设备如磁盘、网卡、显卡等。cpu可以从为一个进程工作，切换到为另一个进程工作。

**常见系统调用分类**

```
文件和设备访问类 比如open/close/read/write/chmod等
进程管理类 fork/clone/execve/exit/getpid等
信号类 signal/sigaction/kill 等
内存管理 brk/mmap/mlock等
进程间通信IPC shmget/semget * 信号量，共享内存，消息队列等
网络通信 socket/connect/sendto/sendmsg 等
其他
```

## 二、环境

2.1操作系统环境

​	LINUX系统，笔者使用的是Debian10，其它LINUX也大同小异。

2.1安装strace工具

我们在系统中运行命令：

```
root@debian:~# strace
-bash: strace：未找到命令
```

说明系统还没安装strace命令。

在官网下载strace软件包，并安装。

https://packages.debian.org/zh-cn/jessie/amd64/strace/download

```
root@debian:~# dpkg -i strace_4.9-2_amd64.deb 
正在选中未选择的软件包 strace。
(正在读取数据库 ... 系统当前共安装有 88338 个文件和目录。)
准备解压 strace_4.9-2_amd64.deb  ...
正在解压 strace (4.9-2) ...
正在设置 strace (4.9-2) ...
正在处理用于 man-db (2.8.5-2) 的触发器 ...
```

其它LINUX版本安装方法有差异，或者可以使用其它方法安装，可以自行查找解决方案，此处不做过多介绍。安装完成后，再次运行strace命令，可以看到strace命令已经可以正常使用了。

```
root@debian:~# strace
usage: strace [-CdffhiqrtttTvVxxy] [-I n] [-e expr]...
              [-a column] [-o file] [-s strsize] [-P path]...
              -p pid... / [-D] [-E var=val]... [-u username] PROG [ARGS]
   or: strace -c[df] [-I n] [-e expr]... [-O overhead] [-S sortby]
              -p pid... / [-D] [-E var=val]... [-u username] PROG [ARGS]
-c -- count time, calls, and errors for each syscall and report summary
-C -- like -c but also print regular output
-w -- summarise syscall latency (default is system time)
-d -- enable debug output to stderr
-D -- run tracer process as a detached grandchild, not as parent
-f -- follow forks, -ff -- with output into separate files
-i -- print instruction pointer at time of syscall
-q -- suppress messages about attaching, detaching, etc.
-r -- print relative timestamp, -t -- absolute timestamp, -tt -- with usecs
-T -- print time spent in each syscall
-v -- verbose mode: print unabbreviated argv, stat, termios, etc. args
-x -- print non-ascii strings in hex, -xx -- print all strings in hex
-y -- print paths associated with file descriptor arguments
-h -- print help message, -V -- print version
-a column -- alignment COLUMN for printing syscall results (default 40)
-b execve -- detach on this syscall
-e expr -- a qualifying expression: option=[!]all or option=[!]val1[,val2]...
   options: trace, abbrev, verbose, raw, signal, read, write
-I interruptible --
   1: no signals are blocked
   2: fatal signals are blocked while decoding syscall (default)
   3: fatal signals are always blocked (default if '-o FILE PROG')
   4: fatal signals and SIGTSTP (^Z) are always blocked
      (useful to make 'strace -o FILE PROG' not stop on ^Z)
-o file -- send trace output to FILE instead of stderr
-O overhead -- set overhead for tracing syscalls to OVERHEAD usecs
-p pid -- trace process with process id PID, may be repeated
-s strsize -- limit length of print strings to STRSIZE chars (default 32)
-S sortby -- sort syscall counts by: time, calls, name, nothing (default time)
-u username -- run command as username handling setuid and/or setgid
-E var=val -- put var=val in the environment for command
-E var -- remove var from the environment for command
-P path -- trace accesses to path
```

至此，环境就准备好了。

## 三、基本命令的使用

查看命令的基本介绍

```
-c -- count time, calls, and errors for each syscall and report summary
//查看报告，包括时间、调用、错误、系统调用名称等
-C -- like -c but also print regular output
//和-c一样，但是同时输出基本信息
-w -- summarise syscall latency (default is system time)
//总结系统延迟，默认使用系统时间
-d -- enable debug output to stderr
//对stderr启用调试输出
-D -- run tracer process as a detached grandchild, not as parent
//将跟踪进程作为分离的子进程运行，而不是作为父进程运行
-f -- follow forks, -ff -- with output into separate files//输出到单独的文件中
//跟踪forks出来的子进程
-i -- print instruction pointer at time of syscall
//在系统调用时打印指令指针
-q -- suppress messages about attaching, detaching, etc.
//禁止显示有关附加、分离等的消息
-r -- print relative timestamp//打印相对时间戳
-t -- absolute timestamp//绝对时间戳
-tt -- with usecs//微妙绝对时间戳
-T -- print time spent in each syscall//打印每个系统调用花费的时间
-v -- verbose mode: print unabbreviated argv, stat, termios, etc. args
//打印未缩写的argv、stat、termios等参数
-x -- print non-ascii strings in hex, -xx -- print all strings in hex
//以十六进制打印非ascii字符串，-xx--以十六进制打印所有字符串
-y -- print paths associated with file descriptor arguments
//打印与文件描述符参数关联的路径
-h -- print help message, -V -- print version
-h—打印帮助消息，—V—打印版本
-a column -- alignment COLUMN for printing syscall results (default 40)
//用于打印系统调用结果的对齐列（默认值为40）
-b execve -- detach on this syscall
//在此系统调用上分离
-e expr -- a qualifying expression: option=[!]all or option=[!]val1[,val2]...
   options: trace, abbrev, verbose, raw, signal, read, write
//限定表达式：option=[！]全部或选项=[！]val1[，val2]。。。
选项：trace，缩写，verbose，raw，signal，read，write
-I interruptible --//可中断的
   1: no signals are blocked//没有信号被阻断
   2: fatal signals are blocked while decoding syscall (default)
   //解码syscall时阻止致命信号（默认）
   3: fatal signals are always blocked (default if '-o FILE PROG')
   //致命信号总是被阻止（如果是'-o FILE PROG'，则默认）
   4: fatal signals and SIGTSTP (^Z) are always blocked
      (useful to make 'strace -o FILE PROG' not stop on ^Z)
//致命信号和SIGTSTP（^Z）始终被阻止
//（有助于使“strace-o FILE PROG”不在^Z停止）
-o file -- send trace output to FILE instead of stderr
//将跟踪输出发送到文件而不是stderr
-O overhead -- set overhead for tracing syscalls to OVERHEAD usecs
//设置跟踪系统调用到开销usec的开销
-p pid -- trace process with process id PID, may be repeated
//进程id为PID的跟踪进程，可以同时多个
-s strsize -- limit length of print strings to STRSIZE chars (default 32)
//将打印字符串的长度限制为STRSIZE字符（默认值为32）
-S sortby -- sort syscall counts by: time, calls, name, nothing (default time)
//syscall计数排序依据：time，calls，name，nothing（默认时间）
-u username -- run command as username handling setuid and/or setgid
//以用户名身份运行命令，处理setuid和/或setgid
-E var=val -- put var=val in the environment for command
//命令环境中的val
-E var -- remove var from the environment for command
//从命令的环境中删除var
-P path -- trace accesses to path
//对路径的跟踪访问
```

接下来，我们做实现，自己编写c程序，通过strace做调试分析。

3.1统计程序发生的系统调用，每个系统调用使用的时间、次数和出错次数等信息。

**实验程序debug_strace_01.c**

```
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
```

编译：gcc -o debug_strace_01 debug_strace_01.c

命令：strace -c ./debug_strace_01

```
% time     seconds  usecs/call     calls    errors syscall
------ ----------- ----------- --------- --------- ----------------
  0.00    0.000000           0         2           read
  0.00    0.000000           0         2           write
  0.00    0.000000           0         3           close
  0.00    0.000000           0         4           fstat
  0.00    0.000000           0         7           mmap
  0.00    0.000000           0         4           mprotect
  0.00    0.000000           0         1           munmap
  0.00    0.000000           0         3           brk
  0.00    0.000000           0         1         1 access
  0.00    0.000000           0         1           execve
  0.00    0.000000           0         1           arch_prctl
  0.00    0.000000           0         3           openat
------ ----------- ----------- --------- --------- ----------------
100.00    0.000000                    32         1 total
```

我们在实验例程中打开了一个文件，往文件中写入一样字符串，读取文件内容，输出到控制台，关闭文件。从strace输出的统计结果中，我们可以看到做了两次read系统调用，两次write系统调用，没有出错。

**补充**

我们尝试把程序中读取部分和往标准输出输出内容部分屏蔽，得到debug_strace_02.c，再次输出统计信息。

```


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
    // while(fgets(buf,512,pf) != NULL)
    // {
    //     printf("%s",buf);
    // }
    // printf("\n");

    fclose(pf);//关闭文件
    return 0;
}
```

编译：gcc -o debug_strace_02 debug_strace_02.c

运行：strace -c ./debug_strace_02

```
% time     seconds  usecs/call     calls    errors syscall
------ ----------- ----------- --------- --------- ----------------
  0.00    0.000000           0         1           read
  0.00    0.000000           0         1           write
  0.00    0.000000           0         3           close
  0.00    0.000000           0         3           fstat
  0.00    0.000000           0         7           mmap
  0.00    0.000000           0         4           mprotect
  0.00    0.000000           0         1           munmap
  0.00    0.000000           0         3           brk
  0.00    0.000000           0         1         1 access
  0.00    0.000000           0         1           execve
  0.00    0.000000           0         1           arch_prctl
  0.00    0.000000           0         3           openat
------ ----------- ----------- --------- --------- ----------------
100.00    0.000000                    29         1 total
```

再次看统计信息，可以看到，程序运行只发生了一次read系统调用和一次write系统调用。说明在读取文件内容和往标准输出输出内容时，发生了一次read系统调用和一次write系统调用。

3.2对stderr启用调试输出

我们还是复用例程debug_strace_01.c

编译：gcc -o debug_strace_01 debug_strace_01.c

命令：strace -d ./debug_strace_01

```
root@debian:~/myGithub/debug_example# strace -d ./debug_strace_01
ptrace_setoptions = 0x11
new tcb for pid 9708, active tcbs:1
 [wait(0x80137f) = 9708] WIFSTOPPED,sig=SIGSTOP,EVENT_STOP (128)
pid 9708 has TCB_STARTUP, initializing it
 [wait(0x80057f) = 9708] WIFSTOPPED,sig=SIGTRAP,EVENT_STOP (128)
 [wait(0x00127f) = 9708] WIFSTOPPED,sig=SIGCONT
 [wait(0x00857f) = 9708] WIFSTOPPED,sig=133
execve("./debug_strace_01", ["./debug_strace_01"], [/* 23 vars */] [wait(0x04057f) = 9708] WIFSTOPPED,sig=SIGTRAP,EVENT_EXEC (4)
 [wait(0x00857f) = 9708] WIFSTOPPED,sig=133
) = 0
 [wait(0x00857f) = 9708] WIFSTOPPED,sig=133
brk(0 [wait(0x00857f) = 9708] WIFSTOPPED,sig=133
)                                  = 0x556416c6c000
 [wait(0x00857f) = 9708] WIFSTOPPED,sig=133
access("/etc/ld.so.preload", R_OK [wait(0x00857f) = 9708] WIFSTOPPED,sig=133
)      = -1 ENOENT (No such file or directory)
 [wait(0x00857f) = 9708] WIFSTOPPED,sig=133
openat(AT_FDCWD, "/etc/ld.so.cache", O_RDONLY|O_CLOEXEC [wait(0x00857f) = 9708] WIFSTOPPED,sig=133
) = 3
 [wait(0x00857f) = 9708] WIFSTOPPED,sig=133
fstat(3,  [wait(0x00857f) = 9708] WIFSTOPPED,sig=133
{st_mode=S_IFREG|0644, st_size=53179, ...}) = 0
 [wait(0x00857f) = 9708] WIFSTOPPED,sig=133
mmap(NULL, 53179, PROT_READ, MAP_PRIVATE, 3, 0 [wait(0x00857f) = 9708] WIFSTOPPED,sig=133
) = 0x7fc3caeb4000
 [wait(0x00857f) = 9708] WIFSTOPPED,sig=133
close(3 [wait(0x00857f) = 9708] WIFSTOPPED,sig=133
)                                = 0
 [wait(0x00857f) = 9708] WIFSTOPPED,sig=133
openat(AT_FDCWD, "/lib/x86_64-linux-gnu/libc.so.6", O_RDONLY|O_CLOEXEC [wait(0x00857f) = 9708] WIFSTOPPED,sig=133
) = 3
 [wait(0x00857f) = 9708] WIFSTOPPED,sig=133
read(3,  [wait(0x00857f) = 9708] WIFSTOPPED,sig=133
"\177ELF\2\1\1\3\0\0\0\0\0\0\0\0\3\0>\0\1\0\0\0\260A\2\0\0\0\0\0"..., 832) = 832
 [wait(0x00857f) = 9708] WIFSTOPPED,sig=133
fstat(3,  [wait(0x00857f) = 9708] WIFSTOPPED,sig=133
{st_mode=S_IFREG|0755, st_size=1824496, ...}) = 0
 [wait(0x00857f) = 9708] WIFSTOPPED,sig=133
mmap(NULL, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0 [wait(0x00857f) = 9708] WIFSTOPPED,sig=133
) = 0x7fc3caeb2000
 [wait(0x00857f) = 9708] WIFSTOPPED,sig=133
mmap(NULL, 1837056, PROT_READ, MAP_PRIVATE|MAP_DENYWRITE, 3, 0 [wait(0x00857f) = 9708] WIFSTOPPED,sig=133
) = 0x7fc3cacf1000
 [wait(0x00857f) = 9708] WIFSTOPPED,sig=133
mprotect(0x7fc3cad13000, 1658880, PROT_NONE [wait(0x00857f) = 9708] WIFSTOPPED,sig=133
) = 0
 [wait(0x00857f) = 9708] WIFSTOPPED,sig=133
mmap(0x7fc3cad13000, 1343488, PROT_READ|PROT_EXEC, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x22000 [wait(0x00857f) = 9708] WIFSTOPPED,sig=133
) = 0x7fc3cad13000
 [wait(0x00857f) = 9708] WIFSTOPPED,sig=133
mmap(0x7fc3cae5b000, 311296, PROT_READ, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x16a000 [wait(0x00857f) = 9708] WIFSTOPPED,sig=133
) = 0x7fc3cae5b000
 [wait(0x00857f) = 9708] WIFSTOPPED,sig=133
mmap(0x7fc3caea8000, 24576, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x1b6000 [wait(0x00857f) = 9708] WIFSTOPPED,sig=133
) = 0x7fc3caea8000
 [wait(0x00857f) = 9708] WIFSTOPPED,sig=133
mmap(0x7fc3caeae000, 14336, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_ANONYMOUS, -1, 0 [wait(0x00857f) = 9708] WIFSTOPPED,sig=133
) = 0x7fc3caeae000
 [wait(0x00857f) = 9708] WIFSTOPPED,sig=133
close(3 [wait(0x00857f) = 9708] WIFSTOPPED,sig=133
)                                = 0
 [wait(0x00857f) = 9708] WIFSTOPPED,sig=133
arch_prctl(ARCH_SET_FS, 0x7fc3caeb3500 [wait(0x00857f) = 9708] WIFSTOPPED,sig=133
) = 0
 [wait(0x00857f) = 9708] WIFSTOPPED,sig=133
mprotect(0x7fc3caea8000, 16384, PROT_READ [wait(0x00857f) = 9708] WIFSTOPPED,sig=133
) = 0
 [wait(0x00857f) = 9708] WIFSTOPPED,sig=133
mprotect(0x556416269000, 4096, PROT_READ [wait(0x00857f) = 9708] WIFSTOPPED,sig=133
) = 0
 [wait(0x00857f) = 9708] WIFSTOPPED,sig=133
mprotect(0x7fc3caee8000, 4096, PROT_READ [wait(0x00857f) = 9708] WIFSTOPPED,sig=133
) = 0
 [wait(0x00857f) = 9708] WIFSTOPPED,sig=133
munmap(0x7fc3caeb4000, 53179 [wait(0x00857f) = 9708] WIFSTOPPED,sig=133
)           = 0
 [wait(0x00857f) = 9708] WIFSTOPPED,sig=133
brk(0 [wait(0x00857f) = 9708] WIFSTOPPED,sig=133
)                                  = 0x556416c6c000
 [wait(0x00857f) = 9708] WIFSTOPPED,sig=133
brk(0x556416c8d000 [wait(0x00857f) = 9708] WIFSTOPPED,sig=133
)                     = 0x556416c8d000
 [wait(0x00857f) = 9708] WIFSTOPPED,sig=133
openat(AT_FDCWD, "./test.txt", O_RDWR|O_CREAT|O_APPEND, 0666 [wait(0x00857f) = 9708] WIFSTOPPED,sig=133
) = 3
 [wait(0x00857f) = 9708] WIFSTOPPED,sig=133
fstat(3,  [wait(0x00857f) = 9708] WIFSTOPPED,sig=133
{st_mode=S_IFREG|0644, st_size=20, ...}) = 0
 [wait(0x00857f) = 9708] WIFSTOPPED,sig=133
write(3, "test\n", 5 [wait(0x00857f) = 9708] WIFSTOPPED,sig=133
)                   = 5
 [wait(0x00857f) = 9708] WIFSTOPPED,sig=133
read(3,  [wait(0x00857f) = 9708] WIFSTOPPED,sig=133
"", 4096)                       = 0
 [wait(0x00857f) = 9708] WIFSTOPPED,sig=133
fstat(1,  [wait(0x00857f) = 9708] WIFSTOPPED,sig=133
{st_mode=S_IFCHR|0600, st_rdev=makedev(136, 0), ...}) = 0
 [wait(0x00857f) = 9708] WIFSTOPPED,sig=133
write(1, "\n", 1
 [wait(0x00857f) = 9708] WIFSTOPPED,sig=133
)                       = 1
 [wait(0x00857f) = 9708] WIFSTOPPED,sig=133
close(3 [wait(0x00857f) = 9708] WIFSTOPPED,sig=133
)                                = 0
 [wait(0x00857f) = 9708] WIFSTOPPED,sig=133
exit_group(0)                           = ?
 [wait(0x000000) = 9708] WIFEXITED,exitcode=0
+++ exited with 0 +++
dropped tcb for pid 9708, 0 remain
```

从报告中，我们不难看出，系统所做了哪些系统调用，返回结果分别是啥，例如

```
close(3 [wait(0x00857f) = 9708] WIFSTOPPED,sig=133
)                                = 0
```

close调用，返回值为0。

3.3跟踪fork出来的子进程

我们修改代码，在代码中使用fork()函数创建子进程，并在子进程中进行往标准输出写数据debug_strace_03.c。

```


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
```

编译：gcc -o debug_strace_03 debug_strace_03.c

命令：strace -f debug_strace_03

```
execve("./debug_strace_03", ["./debug_strace_03"], [/* 23 vars */]) = 0
brk(0)                                  = 0x55682031d000
access("/etc/ld.so.preload", R_OK)      = -1 ENOENT (No such file or directory)
openat(AT_FDCWD, "/etc/ld.so.cache", O_RDONLY|O_CLOEXEC) = 3
fstat(3, {st_mode=S_IFREG|0644, st_size=53179, ...}) = 0
mmap(NULL, 53179, PROT_READ, MAP_PRIVATE, 3, 0) = 0x7f02cabab000
close(3)                                = 0
openat(AT_FDCWD, "/lib/x86_64-linux-gnu/libc.so.6", O_RDONLY|O_CLOEXEC) = 3
read(3, "\177ELF\2\1\1\3\0\0\0\0\0\0\0\0\3\0>\0\1\0\0\0\260A\2\0\0\0\0\0"..., 832) = 832
fstat(3, {st_mode=S_IFREG|0755, st_size=1824496, ...}) = 0
mmap(NULL, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x7f02caba9000
mmap(NULL, 1837056, PROT_READ, MAP_PRIVATE|MAP_DENYWRITE, 3, 0) = 0x7f02ca9e8000
mprotect(0x7f02caa0a000, 1658880, PROT_NONE) = 0
mmap(0x7f02caa0a000, 1343488, PROT_READ|PROT_EXEC, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x22000) = 0x7f02caa0a000
mmap(0x7f02cab52000, 311296, PROT_READ, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x16a000) = 0x7f02cab52000
mmap(0x7f02cab9f000, 24576, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x1b6000) = 0x7f02cab9f000
mmap(0x7f02caba5000, 14336, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_ANONYMOUS, -1, 0) = 0x7f02caba5000
close(3)                                = 0
arch_prctl(ARCH_SET_FS, 0x7f02cabaa500) = 0
mprotect(0x7f02cab9f000, 16384, PROT_READ) = 0
mprotect(0x55681f612000, 4096, PROT_READ) = 0
mprotect(0x7f02cabdf000, 4096, PROT_READ) = 0
munmap(0x7f02cabab000, 53179)           = 0
clone(child_stack=0, flags=CLONE_CHILD_CLEARTID|CLONE_CHILD_SETTID|SIGCHLD, child_tidptr=0x7f02cabaa7d0) = 3497
getpid(Process 3497 attached
)                                = 3496
[pid  3496] fstat(1, {st_mode=S_IFCHR|0600, st_rdev=makedev(136, 0), ...}) = 0
[pid  3497] getpid( <unfinished ...>
[pid  3496] brk(0 <unfinished ...>
[pid  3497] <... getpid resumed> )      = 3497
[pid  3496] <... brk resumed> )         = 0x55682031d000
[pid  3496] brk(0x55682033e000 <unfinished ...>
[pid  3497] fstat(1,  <unfinished ...>
[pid  3496] <... brk resumed> )         = 0x55682033e000
[pid  3496] write(1, "i am the parent process, my proc"..., 48 <unfinished ...>
[pid  3497] <... fstat resumed> {st_mode=S_IFCHR|0600, st_rdev=makedev(136, 0), ...}) = 0
i am the parent process, my process id is 3496/n[pid  3496] <... write resumed> )       = 48
[pid  3496] exit_group(0)               = ?
[pid  3496] +++ exited with 0 +++
brk(0)                                  = 0x55682031d000
brk(0x55682033e000)                     = 0x55682033e000
openat(AT_FDCWD, "./test.txt", O_RDWR|O_CREAT|O_APPEND, 0666) = 3
fstat(3, {st_mode=S_IFREG|0644, st_size=55, ...}) = 0
write(3, "test\n", 5)                   = 5
read(3, "", 4096)                       = 0
write(1, "i am the child process, my proce"..., 48i am the child process, my process id is 3497/n
) = 48
close(3)                                = 0
exit_group(0)                           = ?
+++ exited with 0 +++
```

从报告中，我们可以看到，标记了各个进程pid，和进程所进行的系统调用等信息。

3.4在系统调用时打印指令指针

我们使用debug_strace_01.c进行实验。

编译：gcc -o debug_strace_01 debug_strace_01.c

命令：strace -i ./debug_strace_01

```
[00007f289f5c5a07] execve("./debug_strace_01", ["./debug_strace_01"], [/* 23 vars */]) = 0
[00007f1e45bd87b7] brk(0)               = 0x55c2b25d4000
[00007f1e45bd9417] access("/etc/ld.so.preload", R_OK) = -1 ENOENT (No such file or directory)
[00007f1e45bd94fd] openat(AT_FDCWD, "/etc/ld.so.cache", O_RDONLY|O_CLOEXEC) = 3
[00007f1e45bd9383] fstat(3, {st_mode=S_IFREG|0644, st_size=53179, ...}) = 0
[00007f1e45bd96f3] mmap(NULL, 53179, PROT_READ, MAP_PRIVATE, 3, 0) = 0x7f1e45bb2000
[00007f1e45bd9437] close(3)             = 0
[00007f1e45bd94fd] openat(AT_FDCWD, "/lib/x86_64-linux-gnu/libc.so.6", O_RDONLY|O_CLOEXEC) = 3
[00007f1e45bd95c4] read(3, "\177ELF\2\1\1\3\0\0\0\0\0\0\0\0\3\0>\0\1\0\0\0\260A\2\0\0\0\0\0"..., 832) = 832
[00007f1e45bd9383] fstat(3, {st_mode=S_IFREG|0755, st_size=1824496, ...}) = 0
[00007f1e45bd96f3] mmap(NULL, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x7f1e45bb0000
[00007f1e45bd96f3] mmap(NULL, 1837056, PROT_READ, MAP_PRIVATE|MAP_DENYWRITE, 3, 0) = 0x7f1e459ef000
[00007f1e45bd97a7] mprotect(0x7f1e45a11000, 1658880, PROT_NONE) = 0
[00007f1e45bd96f3] mmap(0x7f1e45a11000, 1343488, PROT_READ|PROT_EXEC, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x22000) = 0x7f1e45a11000
[00007f1e45bd96f3] mmap(0x7f1e45b59000, 311296, PROT_READ, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x16a000) = 0x7f1e45b59000
[00007f1e45bd96f3] mmap(0x7f1e45ba6000, 24576, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x1b6000) = 0x7f1e45ba6000
[00007f1e45bd96f3] mmap(0x7f1e45bac000, 14336, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_ANONYMOUS, -1, 0) = 0x7f1e45bac000
[00007f1e45bd9437] close(3)             = 0
[00007f1e45bc0d9c] arch_prctl(ARCH_SET_FS, 0x7f1e45bb1500) = 0
[00007f1e45bd97a7] mprotect(0x7f1e45ba6000, 16384, PROT_READ) = 0
[00007f1e45bd97a7] mprotect(0x55c2b082a000, 4096, PROT_READ) = 0
[00007f1e45bd97a7] mprotect(0x7f1e45be6000, 4096, PROT_READ) = 0
[00007f1e45bd9787] munmap(0x7f1e45bb2000, 53179) = 0
[00007f1e45adf307] brk(0)               = 0x55c2b25d4000
[00007f1e45adf307] brk(0x55c2b25f5000)  = 0x55c2b25f5000
[00007f1e45ad91ae] openat(AT_FDCWD, "./test.txt", O_RDWR|O_CREAT|O_APPEND, 0666) = 3
[00007f1e45ad8af3] fstat(3, {st_mode=S_IFREG|0644, st_size=60, ...}) = 0
[00007f1e45ad9504] write(3, "test\n", 5) = 5
[00007f1e45ad9461] read(3, "", 4096)    = 0
[00007f1e45ad8af3] fstat(1, {st_mode=S_IFCHR|0600, st_rdev=makedev(136, 0), ...}) = 0
[00007f1e45ad9504] write(1, "\n", 1
)    = 1
[00007f1e45ade4c7] close(3)             = 0
[00007f1e45ab59d6] exit_group(0)        = ?
[????????????????] +++ exited with 0 +++
```

3.5跟踪的每一行都以时间为前缀

使用代码debug_strace_01.c进行实验。

编译：gcc -o debug_strace_01 debug_strace_01.c

命令：strace -t debug_strace_01

```
10:04:22 execve("./debug_strace_01", ["./debug_strace_01"], [/* 23 vars */]) = 0
10:04:22 brk(0)                         = 0x56434607f000
10:04:22 access("/etc/ld.so.preload", R_OK) = -1 ENOENT (No such file or directory)
10:04:22 openat(AT_FDCWD, "/etc/ld.so.cache", O_RDONLY|O_CLOEXEC) = 3
10:04:22 fstat(3, {st_mode=S_IFREG|0644, st_size=53179, ...}) = 0
10:04:22 mmap(NULL, 53179, PROT_READ, MAP_PRIVATE, 3, 0) = 0x7f3aa97ff000
10:04:22 close(3)                       = 0
10:04:22 openat(AT_FDCWD, "/lib/x86_64-linux-gnu/libc.so.6", O_RDONLY|O_CLOEXEC) = 3
10:04:22 read(3, "\177ELF\2\1\1\3\0\0\0\0\0\0\0\0\3\0>\0\1\0\0\0\260A\2\0\0\0\0\0"..., 832) = 832
10:04:22 fstat(3, {st_mode=S_IFREG|0755, st_size=1824496, ...}) = 0
10:04:22 mmap(NULL, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x7f3aa97fd000
10:04:22 mmap(NULL, 1837056, PROT_READ, MAP_PRIVATE|MAP_DENYWRITE, 3, 0) = 0x7f3aa963c000
10:04:22 mprotect(0x7f3aa965e000, 1658880, PROT_NONE) = 0
10:04:22 mmap(0x7f3aa965e000, 1343488, PROT_READ|PROT_EXEC, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x22000) = 0x7f3aa965e000
10:04:22 mmap(0x7f3aa97a6000, 311296, PROT_READ, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x16a000) = 0x7f3aa97a6000
10:04:22 mmap(0x7f3aa97f3000, 24576, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x1b6000) = 0x7f3aa97f3000
10:04:22 mmap(0x7f3aa97f9000, 14336, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_ANONYMOUS, -1, 0) = 0x7f3aa97f9000
10:04:22 close(3)                       = 0
10:04:22 arch_prctl(ARCH_SET_FS, 0x7f3aa97fe500) = 0
10:04:22 mprotect(0x7f3aa97f3000, 16384, PROT_READ) = 0
10:04:22 mprotect(0x5643458ec000, 4096, PROT_READ) = 0
10:04:22 mprotect(0x7f3aa9833000, 4096, PROT_READ) = 0
10:04:22 munmap(0x7f3aa97ff000, 53179)  = 0
10:04:22 brk(0)                         = 0x56434607f000
10:04:22 brk(0x5643460a0000)            = 0x5643460a0000
10:04:22 openat(AT_FDCWD, "./test.txt", O_RDWR|O_CREAT|O_APPEND, 0666) = 3
10:04:22 fstat(3, {st_mode=S_IFREG|0644, st_size=65, ...}) = 0
10:04:22 write(3, "test\n", 5)          = 5
10:04:22 read(3, "", 4096)              = 0
10:04:22 fstat(1, {st_mode=S_IFCHR|0600, st_rdev=makedev(136, 0), ...}) = 0
10:04:22 write(1, "\n", 1
)              = 1
10:04:22 close(3)                       = 0
10:04:22 exit_group(0)                  = ?
10:04:22 +++ exited with 0 +++
```

这个方式，时间使用的是绝对时间，单位精确到秒

3.6如果给出两次，则打印时间包括微妙

命令：strace -tt ./debug_strace_01

```
10:06:41.368903 execve("./debug_strace_01", ["./debug_strace_01"], [/* 23 vars */]) = 0
10:06:41.369787 brk(0)                  = 0x55794dbfe000
10:06:41.370430 access("/etc/ld.so.preload", R_OK) = -1 ENOENT (No such file or directory)
10:06:41.371296 openat(AT_FDCWD, "/etc/ld.so.cache", O_RDONLY|O_CLOEXEC) = 3
10:06:41.372224 fstat(3, {st_mode=S_IFREG|0644, st_size=53179, ...}) = 0
10:06:41.373556 mmap(NULL, 53179, PROT_READ, MAP_PRIVATE, 3, 0) = 0x7f1283588000
10:06:41.374319 close(3)                = 0
10:06:41.375107 openat(AT_FDCWD, "/lib/x86_64-linux-gnu/libc.so.6", O_RDONLY|O_CLOEXEC) = 3
10:06:41.375845 read(3, "\177ELF\2\1\1\3\0\0\0\0\0\0\0\0\3\0>\0\1\0\0\0\260A\2\0\0\0\0\0"..., 832) = 832
10:06:41.376659 fstat(3, {st_mode=S_IFREG|0755, st_size=1824496, ...}) = 0
10:06:41.377480 mmap(NULL, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x7f1283586000
10:06:41.378301 mmap(NULL, 1837056, PROT_READ, MAP_PRIVATE|MAP_DENYWRITE, 3, 0) = 0x7f12833c5000
10:06:41.379109 mprotect(0x7f12833e7000, 1658880, PROT_NONE) = 0
10:06:41.379868 mmap(0x7f12833e7000, 1343488, PROT_READ|PROT_EXEC, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x22000) = 0x7f12833e7000
10:06:41.380809 mmap(0x7f128352f000, 311296, PROT_READ, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x16a000) = 0x7f128352f000
10:06:41.381394 mmap(0x7f128357c000, 24576, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x1b6000) = 0x7f128357c000
10:06:41.382221 mmap(0x7f1283582000, 14336, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_ANONYMOUS, -1, 0) = 0x7f1283582000
10:06:41.382999 close(3)                = 0
10:06:41.383573 arch_prctl(ARCH_SET_FS, 0x7f1283587500) = 0
10:06:41.384323 mprotect(0x7f128357c000, 16384, PROT_READ) = 0
10:06:41.384851 mprotect(0x55794c1e2000, 4096, PROT_READ) = 0
10:06:41.385466 mprotect(0x7f12835bc000, 4096, PROT_READ) = 0
10:06:41.386031 munmap(0x7f1283588000, 53179) = 0
10:06:41.386978 brk(0)                  = 0x55794dbfe000
10:06:41.387997 brk(0x55794dc1f000)     = 0x55794dc1f000
10:06:41.388473 openat(AT_FDCWD, "./test.txt", O_RDWR|O_CREAT|O_APPEND, 0666) = 3
10:06:41.388835 fstat(3, {st_mode=S_IFREG|0644, st_size=70, ...}) = 0
10:06:41.389343 write(3, "test\n", 5)   = 5
10:06:41.389837 read(3, "", 4096)       = 0
10:06:41.390813 fstat(1, {st_mode=S_IFCHR|0600, st_rdev=makedev(136, 0), ...}) = 0
10:06:41.391349 write(1, "\n", 1
)       = 1
10:06:41.391969 close(3)                = 0
10:06:41.392304 exit_group(0)           = ?
10:06:41.392606 +++ exited with 0 +++
```

可以看到，报告上每一行的前面的时间发生了改变，编程了更精确的时间。

3.7如果给定三次，则打印时间包括微妙，并且前导部分将打印为自该1970年1月1日以来的秒数。

命令：strace -ttt ./debug_strace_01

```
1606961600.478551 execve("./debug_strace_01", ["./debug_strace_01"], [/* 23 vars */]) = 0
1606961600.479977 brk(0)                = 0x5594003cd000
1606961600.480524 access("/etc/ld.so.preload", R_OK) = -1 ENOENT (No such file or directory)
1606961600.481279 openat(AT_FDCWD, "/etc/ld.so.cache", O_RDONLY|O_CLOEXEC) = 3
1606961600.481979 fstat(3, {st_mode=S_IFREG|0644, st_size=53179, ...}) = 0
1606961600.482726 mmap(NULL, 53179, PROT_READ, MAP_PRIVATE, 3, 0) = 0x7fa23e8d7000
1606961600.483170 close(3)              = 0
1606961600.483538 openat(AT_FDCWD, "/lib/x86_64-linux-gnu/libc.so.6", O_RDONLY|O_CLOEXEC) = 3
1606961600.483908 read(3, "\177ELF\2\1\1\3\0\0\0\0\0\0\0\0\3\0>\0\1\0\0\0\260A\2\0\0\0\0\0"..., 832) = 832
1606961600.484313 fstat(3, {st_mode=S_IFREG|0755, st_size=1824496, ...}) = 0
1606961600.484753 mmap(NULL, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x7fa23e8d5000
1606961600.485086 mmap(NULL, 1837056, PROT_READ, MAP_PRIVATE|MAP_DENYWRITE, 3, 0) = 0x7fa23e714000
1606961600.485421 mprotect(0x7fa23e736000, 1658880, PROT_NONE) = 0
1606961600.485742 mmap(0x7fa23e736000, 1343488, PROT_READ|PROT_EXEC, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x22000) = 0x7fa23e736000
1606961600.486081 mmap(0x7fa23e87e000, 311296, PROT_READ, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x16a000) = 0x7fa23e87e000
1606961600.486395 mmap(0x7fa23e8cb000, 24576, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x1b6000) = 0x7fa23e8cb000
1606961600.486737 mmap(0x7fa23e8d1000, 14336, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_ANONYMOUS, -1, 0) = 0x7fa23e8d1000
1606961600.487044 close(3)              = 0
1606961600.487484 arch_prctl(ARCH_SET_FS, 0x7fa23e8d6500) = 0
1606961600.487809 mprotect(0x7fa23e8cb000, 16384, PROT_READ) = 0
1606961600.488138 mprotect(0x5593ff292000, 4096, PROT_READ) = 0
1606961600.488455 mprotect(0x7fa23e90b000, 4096, PROT_READ) = 0
1606961600.488766 munmap(0x7fa23e8d7000, 53179) = 0
1606961600.489180 brk(0)                = 0x5594003cd000
1606961600.489506 brk(0x5594003ee000)   = 0x5594003ee000
1606961600.489979 openat(AT_FDCWD, "./test.txt", O_RDWR|O_CREAT|O_APPEND, 0666) = 3
1606961600.490377 fstat(3, {st_mode=S_IFREG|0644, st_size=75, ...}) = 0
1606961600.490896 write(3, "test\n", 5) = 5
1606961600.491357 read(3, "", 4096)     = 0
1606961600.491882 fstat(1, {st_mode=S_IFCHR|0600, st_rdev=makedev(136, 0), ...}) = 0
1606961600.492236 write(1, "\n", 1
)     = 1
1606961600.492555 close(3)              = 0
1606961600.492920 exit_group(0)         = ?
1606961600.493132 +++ exited with 0 +++
```

3.8显示花费在系统调用上的时间。记录每个系统调用的开始和结束之间的时间差。

命令：strace -T ./debug_strace_01

```
execve("./debug_strace_01", ["./debug_strace_01"], [/* 23 vars */]) = 0 <0.000775>
brk(0)                                  = 0x55cd22c64000 <0.000128>
access("/etc/ld.so.preload", R_OK)      = -1 ENOENT (No such file or directory) <0.000248>
openat(AT_FDCWD, "/etc/ld.so.cache", O_RDONLY|O_CLOEXEC) = 3 <0.000311>
fstat(3, {st_mode=S_IFREG|0644, st_size=53179, ...}) = 0 <0.000393>
mmap(NULL, 53179, PROT_READ, MAP_PRIVATE, 3, 0) = 0x7fb1369d8000 <0.000405>
close(3)                                = 0 <0.000107>
openat(AT_FDCWD, "/lib/x86_64-linux-gnu/libc.so.6", O_RDONLY|O_CLOEXEC) = 3 <0.000195>
read(3, "\177ELF\2\1\1\3\0\0\0\0\0\0\0\0\3\0>\0\1\0\0\0\260A\2\0\0\0\0\0"..., 832) = 832 <0.000184>
fstat(3, {st_mode=S_IFREG|0755, st_size=1824496, ...}) = 0 <0.000395>
mmap(NULL, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x7fb1369d6000 <0.000176>
mmap(NULL, 1837056, PROT_READ, MAP_PRIVATE|MAP_DENYWRITE, 3, 0) = 0x7fb136815000 <0.000180>
mprotect(0x7fb136837000, 1658880, PROT_NONE) = 0 <0.000273>
mmap(0x7fb136837000, 1343488, PROT_READ|PROT_EXEC, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x22000) = 0x7fb136837000 <0.000198>
mmap(0x7fb13697f000, 311296, PROT_READ, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x16a000) = 0x7fb13697f000 <0.000357>
mmap(0x7fb1369cc000, 24576, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x1b6000) = 0x7fb1369cc000 <0.000256>
mmap(0x7fb1369d2000, 14336, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_ANONYMOUS, -1, 0) = 0x7fb1369d2000 <0.000234>
close(3)                                = 0 <0.000186>
arch_prctl(ARCH_SET_FS, 0x7fb1369d7500) = 0 <0.000249>
mprotect(0x7fb1369cc000, 16384, PROT_READ) = 0 <0.000239>
mprotect(0x55cd22a0e000, 4096, PROT_READ) = 0 <0.000174>
mprotect(0x7fb136a0c000, 4096, PROT_READ) = 0 <0.000315>
munmap(0x7fb1369d8000, 53179)           = 0 <0.000193>
brk(0)                                  = 0x55cd22c64000 <0.000164>
brk(0x55cd22c85000)                     = 0x55cd22c85000 <0.000172>
openat(AT_FDCWD, "./test.txt", O_RDWR|O_CREAT|O_APPEND, 0666) = 3 <0.000141>
fstat(3, {st_mode=S_IFREG|0644, st_size=80, ...}) = 0 <0.000122>
write(3, "test\n", 5)                   = 5 <0.000266>
read(3, "", 4096)                       = 0 <0.000134>
fstat(1, {st_mode=S_IFCHR|0600, st_rdev=makedev(136, 0), ...}) = 0 <0.000175>
write(1, "\n", 1
)                       = 1 <0.000175>
close(3)                                = 0 <0.000237>
exit_group(0)                           = ?
+++ exited with 0 +++
```

我们可以看到，每个系统调用后方，都统计了使用的时间。

3.9打印环境、统计信息、termios等调用的未缩写版本。

这些结构在调用中非常常见，因此默认行为显示了结构成员的合理子集。使用次选项可以获得所有详细信息。

命令：strace -v ./debug_strace_01

```
execve("./debug_strace_01", ["./debug_strace_01"], ["SHELL=/bin/bash", "LANGUAGE=zh_CN:zh", "PWD=/root/myGithub/debug_example", "LOGNAME=root", "XDG_SESSION_TYPE=tty", "HOME=/root", "LANG=zh_CN.UTF-8", "SSH_CONNECTION=192.168.3.101 522"..., "GOROOT=/usr/local/go", "XDG_SESSION_CLASS=user", "TERM=vt100", "USER=root", "SHLVL=1", "XDG_SESSION_ID=1", "XDG_RUNTIME_DIR=/run/user/0", "SSH_CLIENT=192.168.3.101 52296 2"..., "PATH=/usr/local/sbin:/usr/local/"..., "DBUS_SESSION_BUS_ADDRESS=unix:pa"..., "MAIL=/var/mail/root", "SSH_TTY=/dev/pts/0", "GOPATH=/goWorkPlace", "_=/usr/bin/strace", "OLDPWD=/root/myGithub"]) = 0
brk(0)                                  = 0x56328ee3c000
access("/etc/ld.so.preload", R_OK)      = -1 ENOENT (No such file or directory)
openat(AT_FDCWD, "/etc/ld.so.cache", O_RDONLY|O_CLOEXEC) = 3
fstat(3, {st_dev=makedev(8, 1), st_ino=18350364, st_mode=S_IFREG|0644, st_nlink=1, st_uid=0, st_gid=0, st_blksize=4096, st_blocks=104, st_size=53179, st_atime=2020/12/02-16:38:12, st_mtime=2020/12/01-16:38:08, st_ctime=2020/12/01-16:38:08}) = 0
mmap(NULL, 53179, PROT_READ, MAP_PRIVATE, 3, 0) = 0x7fc903090000
close(3)                                = 0
openat(AT_FDCWD, "/lib/x86_64-linux-gnu/libc.so.6", O_RDONLY|O_CLOEXEC) = 3
read(3, "\177ELF\2\1\1\3\0\0\0\0\0\0\0\0\3\0>\0\1\0\0\0\260A\2\0\0\0\0\0"..., 832) = 832
fstat(3, {st_dev=makedev(8, 1), st_ino=4719229, st_mode=S_IFREG|0755, st_nlink=1, st_uid=0, st_gid=0, st_blksize=4096, st_blocks=3568, st_size=1824496, st_atime=2020/12/03-09:49:40, st_mtime=2019/05/02-01:24:19, st_ctime=2020/08/16-21:47:14}) = 0
mmap(NULL, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x7fc90308e000
mmap(NULL, 1837056, PROT_READ, MAP_PRIVATE|MAP_DENYWRITE, 3, 0) = 0x7fc902ecd000
mprotect(0x7fc902eef000, 1658880, PROT_NONE) = 0
mmap(0x7fc902eef000, 1343488, PROT_READ|PROT_EXEC, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x22000) = 0x7fc902eef000
mmap(0x7fc903037000, 311296, PROT_READ, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x16a000) = 0x7fc903037000
mmap(0x7fc903084000, 24576, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x1b6000) = 0x7fc903084000
mmap(0x7fc90308a000, 14336, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_ANONYMOUS, -1, 0) = 0x7fc90308a000
close(3)                                = 0
arch_prctl(ARCH_SET_FS, 0x7fc90308f500) = 0
mprotect(0x7fc903084000, 16384, PROT_READ) = 0
mprotect(0x56328d5f5000, 4096, PROT_READ) = 0
mprotect(0x7fc9030c4000, 4096, PROT_READ) = 0
munmap(0x7fc903090000, 53179)           = 0
brk(0)                                  = 0x56328ee3c000
brk(0x56328ee5d000)                     = 0x56328ee5d000
openat(AT_FDCWD, "./test.txt", O_RDWR|O_CREAT|O_APPEND, 0666) = 3
fstat(3, {st_dev=makedev(8, 1), st_ino=20584378, st_mode=S_IFREG|0644, st_nlink=1, st_uid=0, st_gid=0, st_blksize=4096, st_blocks=8, st_size=95, st_atime=2020/12/03-10:17:38, st_mtime=2020/12/03-10:17:38, st_ctime=2020/12/03-10:17:38}) = 0
write(3, "test\n", 5)                   = 5
read(3, "", 4096)                       = 0
fstat(1, {st_dev=makedev(0, 20), st_ino=3, st_mode=S_IFCHR|0600, st_nlink=1, st_uid=0, st_gid=5, st_blksize=1024, st_blocks=0, st_rdev=makedev(136, 0), st_atime=2020/12/03-10:18:00, st_mtime=2020/12/03-10:18:00, st_ctime=2020/12/03-09:44:23}) = 0
write(1, "\n", 1
)                       = 1
close(3)                                = 0
exit_group(0)                           = ?
+++ exited with 0 +++
```

3.10限定表达式，用于修改要跟踪的事件或如何跟踪它们。

strace -e expr

3.11跟踪指定的系统调用集。

该-c选项用于确定哪些系统调用可能式跟踪有用。例如

strace -e trace = open,close,read,write表示只跟踪这四个系统调用

strace -e trace=file 跟踪所有以文件名作为参数的系统调用。

命令：strace -e trace=file ./debug_strace_01

```
execve("./debug_strace_01", ["./debug_strace_01"], [/* 23 vars */]) = 0
access("/etc/ld.so.preload", R_OK)      = -1 ENOENT (No such file or directory)
openat(AT_FDCWD, "/etc/ld.so.cache", O_RDONLY|O_CLOEXEC) = 3
openat(AT_FDCWD, "/lib/x86_64-linux-gnu/libc.so.6", O_RDONLY|O_CLOEXEC) = 3
openat(AT_FDCWD, "./test.txt", O_RDWR|O_CREAT|O_APPEND, 0666) = 3

+++ exited with 0 +++
```

strace -e trace=process 跟踪涉及进程管理的所有系统调用。这对于观察进程的派生，等待和执行步骤很有用。

命令：strace -e trace=process ./debug_strace_03

```
execve("./debug_strace_03", ["./debug_strace_03"], [/* 23 vars */]) = 0
arch_prctl(ARCH_SET_FS, 0x7f8a7281b500) = 0
clone(child_stack=0, flags=CLONE_CHILD_CLEARTID|CLONE_CHILD_SETTID|SIGCHLD, child_tidptr=0x7f8a7281b7d0) = 4788
i am the child process, my process id is 4788/n
--- SIGCHLD {si_signo=SIGCHLD, si_code=CLD_EXITED, si_pid=4788, si_uid=0, si_status=0, si_utime=0, si_stime=0} ---
i am the parent process, my process id is 4787/nexit_group(0)                           = ?
+++ exited with 0 +++
```

strace -e trace=network  跟踪所有与网络相关的系统调用。

命令：strace -e trace=network ping 1234321.com

```
socket(PF_INET, SOCK_DGRAM, IPPROTO_ICMP) = -1 EACCES (Permission denied)
socket(PF_INET, SOCK_RAW, IPPROTO_ICMP) = 3
socket(PF_INET6, SOCK_DGRAM, IPPROTO_ICMPV6) = -1 EACCES (Permission denied)
socket(PF_INET6, SOCK_RAW, IPPROTO_ICMPV6) = 4
socket(PF_LOCAL, SOCK_STREAM|SOCK_CLOEXEC|SOCK_NONBLOCK, 0) = 5
connect(5, {sa_family=AF_LOCAL, sun_path="/var/run/nscd/socket"}, 110) = -1 ENOENT (No such file or directory)
socket(PF_LOCAL, SOCK_STREAM|SOCK_CLOEXEC|SOCK_NONBLOCK, 0) = 5
connect(5, {sa_family=AF_LOCAL, sun_path="/var/run/nscd/socket"}, 110) = -1 ENOENT (No such file or directory)
socket(PF_INET, SOCK_DGRAM|SOCK_CLOEXEC|SOCK_NONBLOCK, IPPROTO_IP) = 5
connect(5, {sa_family=AF_INET, sin_port=htons(53), sin_addr=inet_addr("119.29.29.29")}, 16) = 0
sendmmsg(5, {{{msg_name(0)=NULL, msg_iov(1)=[{"\337\273\1\0\0\1\0\0\0\0\0\0\0071234321\3com\0\0\1\0\1", 29}], msg_controllen=0, msg_flags=0}, 29}, {{msg_name(0)=NULL, msg_iov(1)=[{"\205\300\1\0\0\1\0\0\0\0\0\0\0071234321\3com\0\0\34\0\1", 29}], msg_controllen=0, msg_flags=0}, 29}}, 2, MSG_NOSIGNAL) = 2
recvfrom(5, "\337\273\201\200\0\1\0\2\0\0\0\0\0071234321\3com\0\0\1\0\1\300\f\0"..., 2048, 0, {sa_family=AF_INET, sin_port=htons(53), sin_addr=inet_addr("119.29.29.29")}, [16]) = 74
recvfrom(5, "\205\300\201\200\0\1\0\1\0\0\0\0\0071234321\3com\0\0\34\0\1\300\f\0"..., 65536, 0, {sa_family=AF_INET, sin_port=htons(53), sin_addr=inet_addr("119.29.29.29")}, [16]) = 58
socket(PF_INET, SOCK_DGRAM, IPPROTO_IP) = 5
connect(5, {sa_family=AF_INET, sin_port=htons(1025), sin_addr=inet_addr("47.91.169.15")}, 16) = 0
getsockname(5, {sa_family=AF_INET, sin_port=htons(55554), sin_addr=inet_addr("192.168.3.80")}, [16]) = 0
setsockopt(3, SOL_RAW, ICMP_FILTER, ~(ICMP_ECHOREPLY|ICMP_DEST_UNREACH|ICMP_SOURCE_QUENCH|ICMP_REDIRECT|ICMP_TIME_EXCEEDED|ICMP_PARAMETERPROB), 4) = 0
setsockopt(3, SOL_IP, IP_RECVERR, [1], 4) = 0
setsockopt(3, SOL_SOCKET, SO_SNDBUF, [324], 4) = 0
setsockopt(3, SOL_SOCKET, SO_RCVBUF, [65536], 4) = 0
getsockopt(3, SOL_SOCKET, SO_RCVBUF, [131072], [4]) = 0
PING overdue.aliyun.com (47.91.169.15) 56(84) bytes of data.
setsockopt(3, SOL_SOCKET, SO_TIMESTAMP, [1], 4) = 0
setsockopt(3, SOL_SOCKET, SO_SNDTIMEO, "\1\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0", 16) = 0
setsockopt(3, SOL_SOCKET, SO_RCVTIMEO, "\1\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0", 16) = 0
sendto(3, "\10\0\211\202\23w\0\1\256L\310_\0\0\0\0!\206\4\0\0\0\0\0\20\21\22\23\24\25\26\27"..., 64, 0, {sa_family=AF_INET, sin_port=htons(0), sin_addr=inet_addr("47.91.169.15")}, 16) = 64
recvmsg(3, {msg_name(16)={sa_family=AF_INET, sin_port=htons(0), sin_addr=inet_addr("47.91.169.15")}, msg_iov(1)=[{"E\0\0Tz\267@\0Y\1\n\217/[\251\17\300\250\3P\0\0\221\202\23w\0\1\256L\310_"..., 192}], msg_controllen=32, {cmsg_len=32, cmsg_level=SOL_SOCKET, cmsg_type=0x1d /* SCM_??? */, ...}, msg_flags=0}, 0) = 84
socket(PF_INET, SOCK_DGRAM|SOCK_CLOEXEC|SOCK_NONBLOCK, IPPROTO_IP) = 5
connect(5, {sa_family=AF_INET, sin_port=htons(53), sin_addr=inet_addr("119.29.29.29")}, 16) = 0
sendto(5, "\315\270\1\0\0\1\0\0\0\0\0\0\00215\003169\00291\00247\7in-add"..., 43, MSG_NOSIGNAL, NULL, 0) = 43
recvfrom(5, "\315\270\201\203\0\1\0\0\0\1\0\0\00215\003169\00291\00247\7in-add"..., 1024, 0, {sa_family=AF_INET, sin_port=htons(53), sin_addr=inet_addr("119.29.29.29")}, [16]) = 114
64 bytes from 47.91.169.15 (47.91.169.15): icmp_seq=1 ttl=89 time=20.6 ms
sendto(3, "\10\0\257i\23w\0\2\257L\310_\0\0\0\0\372\235\4\0\0\0\0\0\20\21\22\23\24\25\26\27"..., 64, 0, {sa_family=AF_INET, sin_port=htons(0), sin_addr=inet_addr("47.91.169.15")}, 16) = 64
recvmsg(3, {msg_name(16)={sa_family=AF_INET, sin_port=htons(0), sin_addr=inet_addr("47.91.169.15")}, msg_iov(1)=[{"E\0\0Tz\267@\0Y\1\n\217/[\251\17\300\250\3P\0\0\267i\23w\0\2\257L\310_"..., 192}], msg_controllen=32, {cmsg_len=32, cmsg_level=SOL_SOCKET, cmsg_type=0x1d /* SCM_??? */, ...}, msg_flags=0}, 0) = 84
64 bytes from 47.91.169.15 (47.91.169.15): icmp_seq=2 ttl=89 time=14.8 ms
sendto(3, "\10\0\354]\23w\0\3\260L\310_\0\0\0\0\274\250\4\0\0\0\0\0\20\21\22\23\24\25\26\27"..., 64, 0, {sa_family=AF_INET, sin_port=htons(0), sin_addr=inet_addr("47.91.169.15")}, 16) = 64
recvmsg(3, {msg_name(16)={sa_family=AF_INET, sin_port=htons(0), sin_addr=inet_addr("47.91.169.15")}, msg_iov(1)=[{"E\0\0Tz\267@\0Y\1\n\217/[\251\17\300\250\3P\0\0\364]\23w\0\3\260L\310_"..., 192}], msg_controllen=32, {cmsg_len=32, cmsg_level=SOL_SOCKET, cmsg_type=0x1d /* SCM_??? */, ...}, msg_flags=0}, 0) = 84
64 bytes from 47.91.169.15 (47.91.169.15): icmp_seq=3 ttl=89 time=21.8 ms
^CProcess 4983 detached

--- overdue.aliyun.com ping statistics ---
3 packets transmitted, 3 received, 0% packet loss, time 11ms
rtt min/avg/max/mdev = 14.819/19.075/21.781/3.046 ms
```

strace -e trace=signal跟踪所有与信号相关的系统调用。

修改代码让程序不自动退出，通过发送信号杀死进程退出，跟踪信号，debug_strace_04.c

```


//通过c程序调试strace的命令使用
//1.strace -c 
//调试输出统计信息：系统调用名称  次数  出错次数  耗时  耗时占比
#include <stdio.h>
#include <unistd.h>


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

    while(1)
    {
        sleep(2);
    }
    return 0;
}
```

命令：strace -e trace=signal 

新建一个新的终端，查询进程的id，并给进程发送信号  kill -2 pid

```
root@debian:~/myGithub/debug_example# strace -e trace=signal ./debug_strace_04

--- SIGINT {si_signo=SIGINT, si_code=SI_USER, si_pid=5786, si_uid=0} ---
+++ killed by SIGINT +++
```

strace能够追踪到进程收到了信号SIGINT。

strace -e trace=ipc跟踪所有与ipc相关的系统调用。

Linux下的ipc机制有管道、信号、消息队列、信号灯、共享内存和socket等。

debug_strace_05.c

```


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
```

编译：gcc -o debug_strace_05 debug_strace_05.c

命令：strace -e trace=ipc ./debug_strace_05

```
root@debian:~/myGithub/debug_example# strace -e trace=ipc ./debug_strace_05
--- SIGINT {si_signo=SIGINT, si_code=SI_USER, si_pid=6939, si_uid=0} ---
+++ killed by SIGINT +++
```

strace -o 文件名   将跟踪输出写入文件名而不是stderr

strace -p pid 使用进程ID pid附加到该进程并开始跟踪。跟踪可以随时通过键盘中断信号（CTRL-C）终止。

先运行程序debug_strace_04

ps x|grep debug_strace_04查找进程

```
root@debian:~# ps x|grep debug_strace_04
  7177 pts/0    S+     0:00 ./debug_strace_04
  7182 pts/1    S+     0:00 grep debug_strace_04
```

strace -p 7177   附着到进程中进行跟踪。

```
Process 7177 attached
restart_syscall(<... resuming interrupted call ...>) = 0
nanosleep({2, 0}, 0x7ffd78b00cb0)       = 0
nanosleep({2, 0}, ^CProcess 7177 detached
 <detached ...>
```

## 四、命令的组合使用

