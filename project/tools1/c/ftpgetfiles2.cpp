#include"_ftp.h"
#include"_public.h"

CLogFile logfile;

Cftp ftp;

struct st_arg
{
  char host[31];              // 远程服务器的IP和端口。
  int mode;                   // 传输模式，1-被动模式，2-主动模式，缺省采用被动模式。
  char username[31];          // 远程服务器的ftp的用户名。
  char password[31];          // 远程服务器的ftp的密码。
  char remotepath[301];       // 远程服务器存放文件的目录。
  char localpath[301];        // 本地文件存放的目录。
  char matchname[101];        // 待下载文件匹配的规则。
  char listfilename[301];     // 下载前列出服务器文件名的文件。
};
struct st_arg starg;
 
struct st_fileinfo
{
  char filename[301];   // 文件名。
  char mtime[21];       // 文件时间。
};

vector<struct st_fileinfo> vlistfile;
// 存放下载前列出服务器文件名的窗口。



// 处理信号2、15函数。
void EXIT(int sig);

void _help(); // 给出帮助文档。

// 把xml解析到参数starg结构体中。
bool _xmltoarg(char *strxmlbuffer);

// 下载文件功能的函数。
bool _ftpgetfiles();

// 把ftp.nlist()方法获取到的list文件加载到vlistfile中。
bool LoadListFile();

int main(int argc, char *argv[])
{
  // 第一步，把服务器上某目录的文件全部下载到本地目录（可以指定文件名匹配的规则）。
  // 日志文件名，ftp服务器的IP和端口 传输模式（主动/被动） ftp用户名 ftp的密码。
  // 服务器存放文件的目录 本地存放文件的目录 下载文件名的匹配规则。
  if (argc != 3)
  {
    _help();
    return -1;
  } 
  // 处理程序的退出信号。
  // CloseIOAndSignal();
  signal(SIGINT, EXIT);
  signal(SIGTERM, EXIT);

  // 解析xml，得到程序的运行参数。
  if (_xmltoarg(argv[2]) == false)
  { 
    printf("_xmltoarg failed.\n");
    return -1;
  }

  // 小目标，将ftp服务器上某目录的文件下载到本地目录中。
  // 打开日志文件。
  if (logfile.Open(argv[1], "a+") == false)
  {
    logfile.Write("打卡日志文件失败(%s)。\n", argv[1]);
    return -1;
  }

  // 登录ftp服务器。
  if (ftp.login(starg.host, starg.username, starg.password, starg.mode) == false)
  {
    logfile.Write("ftp.login(%s,%s,%s) failed.\n",starg.host,starg.username,starg.password);
    return -1;
  }
  logfile.Write("ftp.login ok.\n");

  _ftpgetfiles();

  // 退出ftp
  ftp.logout();

  return 0;
}

void EXIT(int sig)
{
  printf("程序退出，sig = %d。\n\n", sig);

  exit(0);
}

void _help()
{
  printf("\n");
  printf("Using: /root/project/tools1/bin/ftpgetfiles logfilename xmlbuffer\n\n");

  printf("Example: /root/project/tools1/bin/procctl 30 /root/project/tools1/bin/ftpgetfiles /log/idc/ftpgetfiles_surfdata.log \"<host>192.168.188.100:21</host><mode>1</mode><username>root</username><password>123456</password><localpath>/idcdata/surfdata</localpath><remotepath>/tmp/idc/surfdata</remotepath><matchname>SURF_ZH*.XML,SURF_ZH*.CSV</matchname><listfilename>/idcdata/ftplist/ftpgetfiles_surfdata.list</listfilename>\"\n\n\n");
    
  printf("本程序是通用的功能模块，用于把远程ftp服务器的文件下载到本地目录。\n");
  printf("logfilename是本程序运行的日志文件。\n");
  printf("xmlbuffer为文件下载的参数，如下：\n");
  printf("<host>192.168.188.100:21</host>\n");
  printf("<mode>1</mode>传输模式：1-被动模式，2-主动模式，缺省采用被动模式。\n");
  printf("<username>root</username> 远程服务器ftp的用户名。\n");
  printf("<password>123456</password> 远程服务器ftp的密码。\n");
  printf("<localpath>/idcdata/surfdata</localpath>本地文件存放的目录。\n");
  printf("<remotepath>/tmp/idc/surfdata</remotepath>远程服务器存放的目录。\n");
  printf("<matchname>SURF_ZH*.XML,SURF_ZH*.CSV</matchname>待下载文件匹配规则。\n");
  printf("不匹配的文件不会被下载，本字段尽可能精确，不建议用*匹配全部的文件。\n");
  printf("<listfilename>/idcdata/ftplist/ftpgetfiles_surfdata.list</listfilename> 下载前列出服务器文件名的文件。\n\n\n");
}

// 把xml解析到参数starg结构体中。
bool _xmltoarg(char *strxmlbuffer)
{
  // 解析xml，得到程序运行的参数。
  memset(&starg, 0, sizeof(struct st_arg));

  GetXMLBuffer(strxmlbuffer, "host", starg.host, 30);
  if (strlen(starg.host) == 0)
  {
    logfile.Write("host if null.\n");
    return false;
  }

  GetXMLBuffer(strxmlbuffer, "mode", &starg.mode);
  if (starg.mode != 2)
    starg.mode = 1;  // 只要不是2-主动模式，其余值均为被动模式。

  GetXMLBuffer(strxmlbuffer, "username", starg.username, 30);
  if (strlen(starg.username) == 0)
  {
    logfile.Write("username if null.\n");
    return false;
  }

  GetXMLBuffer(strxmlbuffer, "password", starg.password, 30);
  if (strlen(starg.password) == 0)
  {
    logfile.Write("password if null.\n");
    return false;
  }

  GetXMLBuffer(strxmlbuffer, "remotepath", starg.remotepath, 300);
  if (strlen(starg.remotepath) == 0)
  {
    logfile.Write("remotepath if null.\n");
    return false;
  }

  GetXMLBuffer(strxmlbuffer, "localpath", starg.localpath, 300);
  if (strlen(starg.localpath) == 0)
  {
    logfile.Write("localpath if null.\n");
    return false;
  }

  GetXMLBuffer(strxmlbuffer, "matchname", starg.matchname, 100);
  if (strlen(starg.matchname) == 0)
  {
    logfile.Write("matchname if null.\n");
    return false;
  }
  
  GetXMLBuffer(strxmlbuffer, "listfilename", starg.listfilename, 300);
  if (strlen(starg.listfilename) == 0)
  { 
    logfile.Write("listfilename is null.\n");
    return false;
  }
  return true;
}

// 下载文件功能的函数。
bool _ftpgetfiles()
{
  // 进入ftp服务器存放文件的目录。
  if (ftp.chdir(starg.remotepath) == false)
  {
    logfile.Write("ftp.chdir(%s) failes.\n", starg.remotepath);
    return false;
  }
  
  // 调用ftp.nlist()方法列出服务器目录中的文件，结果存放到本地文件中。
  if (ftp.nlist(".", starg.listfilename) == false)
  {
    logfile.Write("ftp.nlist(%s) failed.\n", starg.listfilename);
    return false;
  } 

  // 把ftp.nlist()方法获取到的list文件加载到容器vlistfile()中。
  if (LoadListFile() == false)
  {
    logfile.Write("LoadListFile() failed.\n");
    return false; 
  }

  // 遍历vlistfile。
  char strremotefilename[301], strlocalfilename[301];
  for (int ii = 0; ii < vlistfile.size(); ii++)
  {
    SNPRINTF(strremotefilename, sizeof(strremotefilename), 300, "%s/%s", starg.remotepath, vlistfile[ii].filename);
    SNPRINTF(strlocalfilename, sizeof(strlocalfilename), 300, "%s/%s", starg.localpath, vlistfile[ii].filename);
    // 调用ftp.get()方法从服务器下载文件。
    logfile.Write("get %s ...", strremotefilename);

    if (ftp.get(strremotefilename, strlocalfilename) == false) 
    {
      logfile.WriteEx("failed.\n");
      break;
    }
    logfile.WriteEx("ok.\n");
  }
  return true;  
}

bool LoadListFile()
{
  vlistfile.clear();

  CFile File;
 
  if (File.Open(starg.listfilename, "r") == false)
  {
    logfile.Write("File.Open(%s) failed.\n", starg.listfilename);
    return false;
  }
  
  struct st_fileinfo stfileinfo;

  while(true)
  {
    memset(&stfileinfo, 0, sizeof(struct st_fileinfo));
    if (File.Fgets(stfileinfo.filename, 300, true) == false)
      break;
    if (MatchStr(stfileinfo.filename, starg.matchname) == false)
      continue;
    vlistfile.push_back(stfileinfo);
  }
  /* 用于测试是否将其加载到vlistfile容器中。
  for (int ii = 0; ii < vlistfile.size(); ii++)
    logfile.Write("filename=%s\n", vlistfile[ii].filename);
  */

  return true;
}
