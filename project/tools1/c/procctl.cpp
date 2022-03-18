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
    printf("Example: /root/project/tools1/bin/procctl 5 /usr/bin/tra zcvf /tmp/tmp.tgz /usr/includ \n\n");

    printf("本程序是服务程序的调度程序，周期性启动服务程序或shell脚本。\n");
    printf("timetvl  运行周期。单位：秒。被调度的程序运行结束后，会在timetvl秒被procctl重新启动。\n");
    printf("program  被调动的程序名称，必须使用全路径。\n");
    printf("argvs    被调动程序的参数。\n");
    printf("注意，本程序不会被kill杀死，但可以使用kill -9命令强行杀死。\n");
    return -1;
  }

  // 关闭信号和和IO，本程序不希望被打扰。
  for (int i = 0; i <= 64; i++)
  {
    signal(i, SIG_IGN);  
    close(i);
  }

  // 生成子进程，父进程退出，让程序在后台运行，由系统1号进程托管。
  if (fork() != 0) exit(0);

  // 启用SIGCHLD信号，让父进程可以wait子进程退出状态。
  signal(SIGCHLD, SIG_DFL);
  
  char *pargv[argc];
  for (int ii = 2; ii < argc; ii++)
    pargv[ii - 2] = argv[ii];
  pargv[argc - 2] = NULL;

  while(true)
  {
    if (fork() == 0)
    {
      execv(argv[2], pargv);
      exit(0); // 这行代码在execv执行成功不会执行，在失败时执行。
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

