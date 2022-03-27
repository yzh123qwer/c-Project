#include "_public.h"

// 程序运行的参数结构体。
struct st_arg
{
  int clienttype;             // 客户端类型，1-上传文件，2-下载文件。
  char ip[31];                // 服务端的IP地址。
  int port;                   // 服务端的接口。
  int ptype;                  // 文件上传成功后文件的处理方式：1-删除文件，2-移动备份目录。
  char clientpath[301];       // 本地文件存放的根目录。
  char clientpathbak[301];    // 文件上传成功后，本地文件的备份目录，但ptype=2时有效。
  bool andchild;              // 是否上传clientpath目录下各级子目录的文件，true-是,false-否 
  char matchname[301];        // 待上传文件名的匹配方式，如“*.TXT,*.XML”。
  char srvpath[301];          // 服务端文件存放的根目录。
  int timetvl;                // 扫描本地目录的时间间隔，单位：秒。
  int timeout;                // 进程心跳的超时时间。
  char pname[51];             // 进程名，建议用“tcpgetfiles_后缀”的方式。
};
struct st_arg starg;

char strrecvbuffer[1024];  // 接受报文的buffer。
char strsendbuffer[1024];  // 发送报文的buffer。

CTcpClient TcpClient;  // 客户端的Tcp操作类。
CPActive PActive;      // 进程心跳。
CLogFile logfile;     // 程序的日志文件。

bool ActiveTest();    
// 心跳。

bool Login(const char *argv);    
// 登录业务。

void _help();
// 程序的帮助文档。

bool _xmltoarg(char *strxmlbuffer);
// 把xml解析到参数starg结构。

void EXIT(int sig);
// 程序退出的信号2,15处理函数。 

// 文件上传的主函数。
bool _tcpputfiles();

// 将文件内容发送到服务端。
bool SendFile(const int sockfd, const char *filename, const int filesize);

// 删除或者转存本地的文件。
bool AckMessage(const char * strrecvbuffer);

int main(int argc,char *argv[])
{
  if (argc!=3)
  {
    _help();
    return -1;
  }
 
  // 关闭全部信号和输入输出。设置2,15信号的处理方式。
  // CloseIOAndSignal();
  signal(2, EXIT);
  signal(15, EXIT);

  if (logfile.Open(argv[1], "a+") == false)
  {
    printf("打开日志文件失败。%s\n", argv[1]);
    return -1;
  }

  if (_xmltoarg(argv[2]) == false)
  {
    logfile.Write("_xmltorget failed!\n");
    return -1;
  }

  PActive.AddPInfo(starg.timeout, starg.pname);  // 将进程信息写入共享内存。

  // 向服务端发起连接请求。
  if (TcpClient.ConnectToServer(starg.ip, starg.port)==false)
  {
    logfile.Write("TcpClient.ConnectToServer(%s,%d) failed.\n",starg.ip, starg.port); 
    EXIT(-1);
  }

  // 登录业务。
  if (Login(argv[2])==false) 
  { 
    logfile.Write("Login failed.\n"); 
    EXIT(-1);
  }

  while(true)
  {
    // 调用文件上传的主函数，执行一次上传文件的任务。
    if (_tcpputfiles() == false)
    {
      logfile.Write("_tcpputfiles() failed.\n");
      EXIT(-1);
    }

    sleep(starg.timetvl);

    if (ActiveTest() == false)
      break;
  }

  EXIT(0);
}

// 心跳。 
bool ActiveTest()    
{
  memset(strsendbuffer, 0, sizeof(strsendbuffer));
  memset(strrecvbuffer, 0, sizeof(strrecvbuffer));

  SPRINTF(strsendbuffer, sizeof(strsendbuffer), "<activetest>ok</activetest>");
  // logfile.Write("发送：%s\n",strsendbuffer);
  if (TcpClient.Write(strsendbuffer) == false) 
    return false; // 向服务端发送请求报文。

  if (TcpClient.Read(strrecvbuffer, 20) == false) 
    return false; // 接收服务端的回应报文。
  // logfile.Write("接收：%s\n",strrecvbuffer);

  return true;
}

// 登录业务。 
bool Login(const char *argv)    
{
  memset(strsendbuffer, 0, sizeof(strsendbuffer));
  memset(strrecvbuffer, 0, sizeof(strrecvbuffer));

  SPRINTF(strsendbuffer,sizeof(strsendbuffer),"%s<clienttype>1</clienttype>", argv);

  logfile.Write("发送：%s\n", strsendbuffer);

  if (TcpClient.Write(strsendbuffer) == false) 
    return false; // 向服务端发送请求报文。

  if (TcpClient.Read(strrecvbuffer, 20) == false) 
    return false; // 接收服务端的回应报文。
  logfile.Write("接受：%s\n", strrecvbuffer);
 
  logfile.Write("Login(%s:%d) ok!\n", starg.ip, starg.port);

  return true;
}

void _help()
{
  printf("\n");
  printf("Using: /root/project/tools1/bin/tcpputfiles logfilename xmlbuffer\n\n");
  printf("Example: /root/project/tools1/bin/procctl 20 /root/project/tools1/bin/tcpputfiles /log/idc/tcpputfiles_surfdata.log \"<ip>192.168.188.100</ip><port>5005</port><ptype>1</ptype><clientpath>/tmp/tcp/surfdata1</clientpath><clientpathbak>/tmp/tcp/surfdata1bak</clientpathbak><andchild>true</andchild><matchname>*.XML,*.CSV</matchname><srvpath>/tmp/tcp/surfdata2</srvpath><timetvl>10</timetvl><timeout>50</timeout><pname>tcpputfiles_surfdata</pname>\"\n");
  printf("         /root/project/tools1/bin/procctl 20 /root/project/tools1/bin/tcpputfiles /log/idc/tcpputfiles_surfdata.log \"<ip>192.168.188.100</ip><port>5005</port><ptype>2</ptype><clientpath>/tmp/tcp/surfdata1</clientpath><clientpathbak>/tmp/tcp/surfdata1bak</clientpathbak><andchild>true</andchild><matchname>*.XML,*.CSV</matchname><srvpath>/tmp/tcp/surfdata2</srvpath><timetvl>10</timetvl><timeout>50</timeout><pname>tcpputfiles_surfdata</pname>\"\n\n\n");
  
  printf("本程序是数据中心的公共功能模块，采用tcp协议把文件发送给服务器。\n");
  printf("logfilename   本程序运行的日志文件。\n");
  printf("xmlbuffer     本程序运行的参数，如下：\n");
  printf("ip            服务端的IP地址。\n");
  printf("port          服务端的端口。\n");
  printf("ptype         文件上传成功后的处理方式，1-删除文件，2-移动备份目录。\n");
  printf("clientpath    本地文件存放的根目录。\n");
  printf("clientpathbak 文件上传成功后，本地文件的备份目录，但ptype=2时有效。\n");
  printf("andchild      是否上传clientpath目录下各级子目录的文件，true-是,false-否。\n");
  printf("matchname     待上传文件名的匹配方式，如“*.TXT,*.XML”。\n");
  printf("srvpath       服务端文件存放的根目录。\n");
  printf("timetvl       扫描本地目录的时间间隔，单位：秒。\n");
  printf("timeout       进程心跳的超时时间。\n");
  printf("pname         进程名，建议用“tcpgetfiles_后缀”的方式。\n\n\n");
}


bool _xmltoarg(char *strxmlbuffer) 
{
  memset(&starg, 0, sizeof(struct st_arg));
  
  GetXMLBuffer(strxmlbuffer, "ip", starg.ip);
  if (strlen(starg.ip) == 0)
  {
    logfile.Write("ip is null.\n");
    return false;
  }
  
  GetXMLBuffer(strxmlbuffer, "port", &starg.port);
  if (starg.port == 0)
  {
    logfile.Write("port is null.\n");
    return false;
  }
  
  GetXMLBuffer(strxmlbuffer, "ptype", &starg.ptype);
  if ((starg.ptype != 1) && (starg.ptype != 2))
  {
    logfile.Write("ptype not in (1, 2).\n");
    return false;
  }

  GetXMLBuffer(strxmlbuffer, "clientpath", starg.clientpath);
  if (strlen(starg.clientpath) == 0)
  {
    logfile.Write("clientpath is null.\n");
    return false;
  }
 
  GetXMLBuffer(strxmlbuffer, "clientpathbak", starg.clientpathbak);
  if (strlen(starg.clientpathbak) == 0)
  {
    logfile.Write("clientpathbak is null.\n");
    return false;
  }

  GetXMLBuffer(strxmlbuffer, "andchild", &starg.andchild);
  
  GetXMLBuffer(strxmlbuffer, "matchname", starg.matchname);
  if (strlen(starg.ip) == 0)
  {
    logfile.Write("matchname is null.\n");
    return false;
  }

  GetXMLBuffer(strxmlbuffer, "srvpath", starg.srvpath);
  if (strlen(starg.srvpath) == 0)
  {
    logfile.Write("srvpath is null.\n");
    return false;
  }

  GetXMLBuffer(strxmlbuffer, "timetvl", &starg.timetvl);
  if (starg.timetvl == 0)
  {
    logfile.Write("timetvl is null.\n");
    return false;
  }
  // 扫描本地目录文件时间间隔，单位：秒。
  // starg.timeval没有必要超过30秒。
  if (starg.timetvl > 30)
    starg.timetvl = 30;

  GetXMLBuffer(strxmlbuffer, "timeout", &starg.timeout);
  if (starg.timeout == 0)
  {
    logfile.Write("timeout is null.\n");
    return false;
  }
  // 进程的心跳超时时间，没有必要小于50秒。
  if (starg.timeout < 50)
    starg.timeout = 50;
  
  GetXMLBuffer(strxmlbuffer, "pname", starg.pname);
  if (strlen(starg.pname) == 0)
  {
    logfile.Write("pname is null.\n");
    return false;
  }
  return true;
}

void EXIT(int sig)
{
  logfile.Write("程序退出，sig = %d\n", sig);
  exit(0);
}


bool _tcpputfiles() 
{
  CDir Dir;
  // 调用OpenDir()打开starg.localpath目录。
  if (Dir.OpenDir(starg.clientpath, starg.matchname, 10000, starg.andchild) == false)
  {
    logfile.Write("Dir.OpenDir(%s) failed.\n", starg.clientpath);
    return false;
  }
  while(true)
  {
    memset(strsendbuffer, 0, sizeof(strsendbuffer));
    memset(strrecvbuffer, 0, sizeof(strrecvbuffer));
    // 遍历目录中的每个文件，调用ReadDir()获取一个文件名。
    if (Dir.ReadDir() == false)
      break;  // 读取失败的原因通常是全部读取完了。

    // 把文件名、修改时间、文件大小组成报文，发送给服务端。
    SNPRINTF(strsendbuffer, sizeof(strsendbuffer), 1000, "<filename>%s</filename><mtime>%s</mtime><size>%d</size>", Dir.m_FullFileName, Dir.m_ModifyTime, Dir.m_FileSize);
    // logfile.Write("strsendbuffer = %s\n", strsendbuffer); 
    if (TcpClient.Write(strsendbuffer) == false)
    {
      logfile.Write("TcpClient.Write(%s) failed.\n", strsendbuffer);
      return false;
    }

    // 把文件内容发送给服务端。
    logfile.Write("send %s (%d) ...", Dir.m_FullFileName, Dir.m_FileSize);
    if (SendFile(TcpClient.m_connfd, Dir.m_FullFileName, Dir.m_FileSize) == true)
    {
      logfile.WriteEx("ok.\n");
    }
    else 
    {
      logfile.WriteEx("failed.\n");
      TcpClient.Close();
      return false;
    }
    
    // 接受服务端的确认报文。
    if (TcpClient.Read(strrecvbuffer, 20) == false)
    {
      logfile.Write("TcpClient.Read() failed.\n");
      return false;
    }
    // logfile.Write("strrecvbuffer = %s\n", strrecvbuffer);

    // 删除或者转存本地的文件。
    AckMessage(strrecvbuffer);
  }
  
  return true;
}

bool SendFile(const int sockfd, const char *filename, const int filesize)
{
  int onread = 0;     // 每次调用fread时打算读取的字节数。
  int bytes = 0;      // 调用一次fread从文件中读取的字节数。
  char buffer[1000];  // 存放读取的buffer。
  int totalbytes = 0;     // 从文件中已读取的字节数。
  FILE *fp = NULL;   

  // 以"rb"的模式打开文件。
  if ((fp = fopen(filename, "rb")) == NULL)
  {
    logfile.Write("fopen(%s) failed.\n", filename);
    return false;
  }
  while (true)
  {
    memset(buffer, 0, sizeof(buffer));
    // 计算本次应该读取的字节端，如果剩余的数据超过1000字节，就打算读1000字节。
    if (filesize - totalbytes > 1000)
      onread = 1000;
    else 
      onread = filesize - totalbytes;
    
    // 从文件中读取数据。
    bytes = fread(buffer, 1, onread, fp);
 
    // 把读取到的数据发送给服务端。
    if (bytes > 0)
    {
      if (Writen(sockfd, buffer, bytes) == false)
      {
        logfile.Write("Writen(%s) failed.\n", buffer);
        fclose(fp);
        return false;
      }
    }

    // 计算文件已读取的字节总数，如果读取完成，跳出循环。
    totalbytes = totalbytes + bytes;
    if (totalbytes == filesize)
      break; 
  }
  fclose(fp);
  return true;
} 

bool AckMessage(const char * strrecvbuffer) 
{
  char filename[301];
  char result[11];
  
  memset(filename, 0, sizeof(filename));
  memset(result, 0, sizeof(result));
  GetXMLBuffer(strrecvbuffer, "filename", filename, 300);
  GetXMLBuffer(strrecvbuffer, "result", result, 10);

  if (strcmp(result, "ok") != 0)
    return true;

  logfile.Write("%s  server recive!...", filename);

  if (starg.ptype == 1)
  {
    if (REMOVE(filename) == false)
    {
      logfile.Write("REMOVE(%s) failed.\n", filename);
      return false;
    }
    logfile.WriteEx("remove.\n");
  }
////////////////////////////////////////////////////////////////
  if (starg.ptype == 2)
  {
    char bakfilename[301];
    STRCPY(bakfilename, sizeof(bakfilename), filename);
    logfile.WriteEx("\nbakfilename = %s\n", bakfilename);
    logfile.WriteEx("filename = %s\n", filename);
    logfile.WriteEx("starg.clientpath = %s\n", starg.clientpath);
    logfile.WriteEx("starg.slientpathbak = %s\n", starg.clientpathbak);
    UpdateStr(bakfilename, starg.clientpath, starg.clientpathbak, false);
    logfile.WriteEx("bakfilename = %s", bakfilename);
    if (RENAME(filename, bakfilename) == false)
    {
      logfile.Write("RENAME(%s, %s) failed.\n", filename, bakfilename);
      return false;
    }
    logfile.WriteEx("bak\n");
  }
//////////////////////////////////////////////////////////////////// 
  return true;
}
