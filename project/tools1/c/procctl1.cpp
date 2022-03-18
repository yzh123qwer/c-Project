#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<signal.h>

int main(int argc, char *argv[])
{
  if (argc < 3)
  {
    printf("Using: ./procctl timetvl program argv...\n");
    printf("Example: /root/project/tools1/bin/procctl 5 /usr/bin/ls -lt /tmp\n\n");

    printf("本程序是服务程序的调度程序，周期性启动服务程序或shell脚本。\n");
    printf("timetvl  运行周期。单位：秒。被调度的程序运行结束后，会在timetvl秒被procctl重新启动。\n");
    printf("program  被调动的程序名称，必须使用全路径。\n");
    printf("argvs    被调动程序的参数。\n");
    printf("注意，本程序不会被kill杀死，但可以使用kill -9命令强行杀死。\n");
    return -1;
  }

  for (int i = 0; i <= 64; i++)
    signal(i, SIG_IGN);  

  // 先执行fork函数，创建一个子进程，让子进程调用execl执行新程序。
  // 新程序将替换子进程，不会影响父进程。
  // 在父进程中，可以调用wait函数等待新程序运行的结果，这样就可以实现调度的功能。
  // ./procctl 10 /usr/bin/ls -lt /tmp
  
  /*
  while (true)
  {
    if (fork() == 0)
      execl(argv[2], "argv[2]", argv[3], argv[4], (char*)0);
    else 
    {
      int status;
      wait(&status);
      sleep(atoi(argv[1]));
    }
  }   
  // excel函数用参数中制定的程序替换了当前进程的正文段，数据段，堆和栈。
  这是用excel实现只带一个参数的程序
  */

  char ** pargv = argv + 2; 

  while(true)
  {
    if (fork() == 0)
    {
      execv(argv[2], pargv);
    }
    else
    {
      int status;
      wait(&status);
      sleep(atoi(argv[1]));
    }
  }
  return 0;
}

