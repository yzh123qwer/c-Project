#include "_public.h"

// 程序运行的参数结构体。
struct st_arg
{
  int clienttype;             // 客户端类型，1-上传文件，2-下载文件。
  int ptype;                  // 文件传输成功后文件的处理方式：1-删除文件，2-移动备份目录。
  char clientpath[301];       // 客户端文件存放的根目录。
  char serverpath[301];       // 服务端存放文件的根目录。
  char serverpathbak[301];    // 服务端上传文件后，本地文件的备份目录。
  bool andchild;              // 是否上传clientpath目录下各级子目录的文件，true-是,false-否
  char matchname[301];        // 待上传文件名的匹配方式，如“*.TXT,*.XML”。
  char srvpath[301];          // 服务端文件存放的根目录。
  int timetvl;                // 扫描本地目录的时间间隔，单位：秒。
  int timeout;                // 进程心跳的超时时间。
  char pname[51];             // 进程名，建议用“tcpgetfiles_后缀”的方式。
};
struct st_arg starg;

CLogFile logfile;      // 服务程序的运行日志。
CTcpServer TcpServer;  // 创建服务端对象。

char strrecvbuffer[1024];  // 接受报文的buffer。
char strsendbuffer[1024];  // 发送报文的buffer。

CPActive PActive;    // 进程心跳。

bool _xmltoarg(char *strxmlbuffer);
// 把xml解析到参数starg结构。

void FathEXIT(int sig);  // 父进程退出函数。
void ChildEXIT(int sig);  // 子进程退出函数。

// 登录业务处理函数。
bool ClientLogin();

// 接受上传文件的主函数。
void RecvFileMain();

// 接受上传文件的内容。
bool RecvFile(const int sockfd, const char *filename, const char *mtime, int filesize);

// 接收下载文件的主函数。
bool SendFileMain();

// 接收一次下载文件的主函数。
bool SendFile(const int sockfd, const char *filename, const int filesize);

// 接收下载文件之后，本地文件的处理函数。
bool AckMessage(const char * strrecvbuffer);

int main(int argc,char *argv[])
{
  if (argc != 3)
  {
    printf("Using:./fileserver port logfile\n");
    printf("Example:/root/project/tools1/c/fileserver 5005 /tmp/idc/fileserver.log\n\n"); 
    return -1;
  }

  // 关闭全部的信号和输入输出。
  // 设置信号,在shell状态下可用 "kill + 进程号" 正常终止些进程
  // 但请不要用 "kill -9 +进程号" 强行终止
  // CloseIOAndSignal(); 
  signal(SIGINT,FathEXIT); 
  signal(SIGTERM,FathEXIT);

  if (logfile.Open(argv[2],"a+")==false) 
  { 
    printf("logfile.Open(%s) failed.\n",argv[2]); 
    return -1; 
  }

  // 服务端初始化。
  if (TcpServer.InitServer(atoi(argv[1]))==false)
  {
    logfile.Write("TcpServer.InitServer(%s) failed.\n",argv[1]); 
    return -1;
  }
  logfile.Write("TcpServer.InitServer(%s) ok.\n", argv[1]);

  while (true)
  {
    // 等待客户端的连接请求。
    if (TcpServer.Accept()==false)
    {
      logfile.Write("TcpServer.Accept() failed.\n"); FathEXIT(-1);
    }

    logfile.Write("客户端（%s）已连接。\n",TcpServer.GetIP());

    /*
    if (fork()>0) { TcpServer.CloseClient(); continue; }  // 父进程继续回到Accept()。
   
    // 子进程重新设置退出信号。
    signal(SIGINT,ChildEXIT); signal(SIGTERM,ChildEXIT);

    TcpServer.CloseListen();
    */
    
    // 子进程与客户端进行通讯，处理业务。
  
    // 处理登录客户端的登录报文。
    if (ClientLogin() == false)
      ChildEXIT(-1);

    // 如果clienttype == 1，调用上传文件的主函数。
    if (starg.clienttype == 1)
      RecvFileMain();

    // 如果clienttype == 2，调用下载文件的主函数。
    if (starg.clienttype == 2)
      SendFileMain();
  }
}

bool _xmltoarg(char *strxmlbuffer)
{
  memset(&starg, 0, sizeof(struct st_arg));
 
  GetXMLBuffer(strxmlbuffer, "clienttype", &starg.clienttype);
  GetXMLBuffer(strxmlbuffer, "ptype", &starg.ptype);
  GetXMLBuffer(strxmlbuffer, "clientpath", starg.clientpath);
  GetXMLBuffer(strxmlbuffer, "andchild", &starg.andchild);
  GetXMLBuffer(strxmlbuffer, "matchname", starg.matchname);
  GetXMLBuffer(strxmlbuffer, "srvpath", starg.srvpath);
  GetXMLBuffer(strxmlbuffer, "serverpath", starg.serverpath);
  GetXMLBuffer(strxmlbuffer, "serverpathbak", starg.serverpathbak);

  GetXMLBuffer(strxmlbuffer, "timetvl", &starg.timetvl);
  // 扫描本地目录文件时间间隔，单位：秒。
  // starg.timeval没有必要超过30秒。
  if (starg.timetvl > 30)
    starg.timetvl = 30;

  GetXMLBuffer(strxmlbuffer, "timeout", &starg.timeout);
  // 进程的心跳超时时间，没有必要小于50秒。
  if (starg.timeout < 50)
    starg.timeout = 50;

  GetXMLBuffer(strxmlbuffer, "pname", starg.pname, 50);
  strcat(starg.pname, "_srv");
  
  return true;
}


// 父进程退出函数。
void FathEXIT(int sig)  
{
  // 以下代码是为了防止信号处理函数在执行的过程中被信号中断。
  signal(SIGINT,SIG_IGN); signal(SIGTERM,SIG_IGN);

  logfile.Write("父进程退出，sig=%d。\n",sig);

  TcpServer.CloseListen();    // 关闭监听的socket。

  kill(0,15);     // 通知全部的子进程退出。

  exit(0);
}

// 子进程退出函数。
void ChildEXIT(int sig)  
{
  // 以下代码是为了防止信号处理函数在执行的过程中被信号中断。
  signal(SIGINT,SIG_IGN); signal(SIGTERM,SIG_IGN);

  logfile.Write("子进程退出，sig=%d。\n",sig);

  TcpServer.CloseClient();    // 关闭客户端的socket。

  exit(0);
}

// 登录。
bool ClientLogin()
{
  memset(strsendbuffer, 0, sizeof(strsendbuffer));
  memset(strrecvbuffer, 0, sizeof(strrecvbuffer));

  if (TcpServer.Read(strrecvbuffer, 20) == false)
  {
    logfile.Write("TcpServer.Read() failed.\n");
    return false;
  }
  logfile.Write("strrecvbuffer = %s\n", strrecvbuffer);

  // 解析strrecvbuffer，获取业务参数。
  _xmltoarg(strrecvbuffer);

  if (starg.clienttype == 1 || starg.clienttype == 2)
    strcpy(strsendbuffer, "ok");
  else 
    strcpy(strsendbuffer, "failed");
 
  if (TcpServer.Write(strsendbuffer) == false)
  {
    logfile.Write("TepServer.Write(%s)failed.\n", strsendbuffer);
    return false;
  }
  logfile.Write("%s login %s.\n", TcpServer.GetIP(), strsendbuffer);
  
  return true;
}


void RecvFileMain() 
{
  PActive.AddPInfo(starg.timeout, starg.pname);

  while(true)
  {
    memset(strsendbuffer, 0, sizeof(strsendbuffer));
    memset(strrecvbuffer, 0, sizeof(strrecvbuffer));

    PActive.UptATime();

    // 接受客户端的报文。
    if (TcpServer.Read(strrecvbuffer, starg.timetvl + 10) == false)
    {
      logfile.Write("TcpServer.Read() failed.\n"); 
      return;
    } 
    // logfile.Write("strrecvbuffer = %s\n", strrecvbuffer);

    // 处理心跳报文。
    if (strcmp(strrecvbuffer, "<activetest>ok</activetest>") == 0)
    {
      strcpy(strsendbuffer, "ok");
      // logfile.Write("strsendbuffer = %s\n", strsendbuffer);
      if (TcpServer.Write(strsendbuffer) == false)
      {
        logfile.Write("TcpServer.Write(%s) failed.\n", strsendbuffer);
        return;
      }
    }

    // 处理上传文件的请求报文。
    if (strncmp(strrecvbuffer, "<filename>", 10) == 0)
    {
      // 解析上传文件请求报文的xml。
      char clientfilename[301];
      char mtime[21];
      memset(clientfilename, 0, sizeof(clientfilename));
      memset(mtime, 0, sizeof(mtime));
      int filesize = 0;
      GetXMLBuffer(strrecvbuffer, "filename", clientfilename, 300);
      GetXMLBuffer(strrecvbuffer, "mtime", mtime, 19);
      GetXMLBuffer(strrecvbuffer, "size", &filesize);

      // 客户端和服务端文件的目录是不一样的，首先生成服务端的文件名。
      // 把文件名中的clientpath替换成srvpath，注意第三个参数。
      char serverfilename[301]; 
      memset(serverfilename, 0, sizeof(serverfilename));
      strcpy(serverfilename, clientfilename);
      UpdateStr(serverfilename, starg.clientpath, starg.srvpath, false);

      // 接受上传文件的内容。
      logfile.Write("recv %s(%d) ... ", serverfilename, filesize);
      if (RecvFile(TcpServer.m_connfd, serverfilename, mtime, filesize) == true)
      {
        logfile.WriteEx("ok.\n");
        SNPRINTF(strsendbuffer, sizeof(strsendbuffer), 1000, "<filename>%s</filename><result>ok</result>", clientfilename);
      }
      else 
      {
        logfile.WriteEx("failed\n");
        SNPRINTF(strsendbuffer, sizeof(strsendbuffer), 1000, "<filename>%s</filename><result>filed</result>", clientfilename);
      }

      // 把接受结果返回给客户端。
      if (TcpServer.Write(strsendbuffer) == false)
      {
        logfile.Write("TcpServer.Write(%s) failed.\n", strsendbuffer);
        return;
      }
    }
  } 
}

bool RecvFile(const int sockfd, const char *filename, const char *mtime, int filesize) 
{
  // 生成临时文件名。
  char strfilenametmp[301];
  SNPRINTF(strfilenametmp, sizeof(strfilenametmp), 300, "%s.tmp", filename);
  
  int totalbytes = 0;   // 已接收文件的总字节数。
  int onread = 0;       // 本次打算接收的总字节数。
  char buffer[1000];    // 接收文件内容的缓冲区。
  FILE *fp = NULL;

  // 创建临时文件。
  if ((fp = FOPEN(strfilenametmp, "wb")) == NULL)
  {
    logfile.Write("FOPEN(%s) failed.\n", strfilenametmp);
    return false;
  }

  while(true)
  {
    memset(buffer, 0, sizeof(buffer));
    // 计算本次应该接收的字节数。
    if (filesize - totalbytes > 1000)
      onread = 1000;
    else 
      onread = filesize - totalbytes;

    // 接收文件内容。
    if (Readn(sockfd, buffer, onread) == false)
    {
      logfile.Write("Readn() failed.\n");
      fclose(fp);
      return false;
    }

    // 把接收到的内容写入文件。
    fwrite(buffer, 1, onread, fp);

    // 计算已接收的总字节数，如果文件已经接收完，跳出循环。
    totalbytes = totalbytes + onread;
   
    if (totalbytes == filesize)
      break;
  }

  // 关闭临时文件。
  fclose(fp);
  
  // 重置文件的时间。
  UTime(strfilenametmp, mtime); // 文件原始时间的意义大于文件的传输时间。

  // 把临时文件RENAME为正式的文件。
  if (RENAME(strfilenametmp, filename) == false)
    return false;  

  return true;
}

bool SendFileMain() 
{
  CDir Dir;
  if (Dir.OpenDir(starg.serverpath, starg.matchname, 10000, starg.andchild) == false)
  {
    logfile.Write("Dir.Open(%s) failed.\n", starg.serverpath);
    return false;
  }

  int buflen = 0; // 用于记录TcpRead函数结束到的报文的长度。

  int delayed = 0; // 未收到确认报文的文件数量。

  while(true)
  {
    memset(strsendbuffer, 0, sizeof(strsendbuffer));
    memset(strrecvbuffer, 0, sizeof(strrecvbuffer));
   
    if (Dir.ReadDir() == false)
      break;  // 读取失败的原因通常是全部都已经读取完成。
    
    // 把文件名、修改时间、文件大小组成报文，发送到客户端。
    SNPRINTF(strsendbuffer, sizeof(strsendbuffer), 1000, "<filename>%s</filename><mtime>%s</mtime><size>%d</size>", Dir.m_FullFileName, Dir.m_ModifyTime, Dir.m_FileSize);
    logfile.Write("strsendbuffer = %s\n", strsendbuffer);

    if (TcpServer.Write(strsendbuffer) == false)
    {
      logfile.Write("TcpClient.Write(%s) failed.\n", strsendbuffer);
      return false;
    }

    // 把文件内容发送给客户端。
    logfile.Write("send %s(%d)...", Dir.m_FullFileName, Dir.m_FileSize);
    if (SendFile(TcpServer.m_connfd, Dir.m_FullFileName, Dir.m_FileSize) == true)
    {
      delayed++;
      logfile.WriteEx("ok.\n");
    }
    else 
    {
      logfile.WriteEx("failed.\n");
      return false;
    }

    PActive.UptATime();
  
    while(delayed > 0)
    {
      if (TcpRead(TcpServer.m_connfd, strrecvbuffer, &buflen, -1) == false)
        break;
      logfile.Write("strrecvbuffer = %s, buflen = %d.\n", strrecvbuffer, buflen);

      delayed--;  
  
      // 删除或者转存本地文件。
      AckMessage(strrecvbuffer);
    }
  }

  while(delayed > 0)
  {
    memset(strrecvbuffer, 0, sizeof(strrecvbuffer));
    if (TcpRead(TcpServer.m_connfd, strrecvbuffer, &buflen, 0) == false)
      break;
    
    logfile.Write("strrecvbuffer = %s\n", strrecvbuffer);
    
    delayed--;
    AckMessage(strrecvbuffer);
  }
  if (TcpServer.Write("finish!") == false)
  {
    logfile.Write("finish send failed.\n");
    return false;
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

  if (starg.ptype == 2)
  {
    char bakfilename[301];
    STRCPY(bakfilename, sizeof(bakfilename), filename);
    UpdateStr(bakfilename, starg.serverpath, starg.serverpathbak, false);
    logfile.WriteEx("bakfilename = %s", bakfilename);
    if (RENAME(filename, bakfilename) == false)
    {
      logfile.Write("RENAME(%s, %s) failed.\n", filename, bakfilename);
      return false;
    }
    logfile.WriteEx("bak\n");
  }
  return true;
}










