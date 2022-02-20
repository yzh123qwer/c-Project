/*
 * 程序名：crtsurfdata1.cpp 本程序用于生成全国气象站点每分钟生成的数据
 * 作者：姚子涵
*/

#include"_public.h"
CLogFile logfile;
int main(int argc, char *argv[])
{
  // inifile outpath logfile
  if (argc != 4)
  {
    printf("Using:./crtsurfdatal infile outpath logfile\n");
    printf("Example:/root/project/idc1/bin/crtsurfdata1 /root/project/idc1/ini/stcode.ini /tmp/surfdata /log/idc/crtsurfdata1.log\n\n");

    printf("infile   全国气象站点参数文件名。\n");
    printf("outpath  全国气象站点数据文件存放的目录。\n");
    printf("logfile  本程序运行的日志文件名。\n\n");

    return -1;
  }

  if (logfile.Open(argv[3]) == false)
  {
    printf("logfile.Open(%s) failed.\n", argv[3]);
    return -1;
  }

  logfile.Write("crtsurfdata1 开始运行。\n");
  //这里插入业务代码。
  logfile.Write("crtsurfdata1 运行结束。\n");

  return 0;
}
