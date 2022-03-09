/* 
 * 程序名：crtsurfdata5.cpp 本程序用于生成全国气象站点每分钟生成的数据
 * 作者：姚子涵
*/
#include"_public.h"

bool LoadSTCode(const char *inifile); 
// 把站点参数文件参数加载到vstcode容器中

void CrtSurfData();
// 模拟生成气象观测数据，存放在vsurfdata容器中。

bool CtrSurfFile(const char *outpath, const char *datafmt);
// 把容器vsurfdata中的全国气象站点分钟观测数据写入文件


char strddatetime[21]; // 观测数据时间
struct st_code
{
  char provname[31]; //省名称
  char obtid[11];    //站号
  char obtname[31];  //站名
  double lat;        //纬度
  double lon;        //经度
  double height;     //海拔高度
};
//存放全国气象站点参数的容器
vector<struct st_code> vstcode;

// 全国气象站点没分钟观测数据结构
struct st_surfdata
{
  char obtid[11];      // 站点代码、
  char ddatetime[21];  // 数据时间：格式yyyymmddhh24miss
  int t;               // 气温：单位0.1摄氏度
  int p;               // 气压：0.1百帕
  int u;               // 相对湿度：0-100之间的值
  int wd;              // 风向：0-360之间的值
  int wf;              // 风速：单位0.1m/s
  int r;               // 降雨量：0.1mm
  int vis;             // 能见度：0.1米
};
vector<struct st_surfdata> vsurfdata; 
// 存放全国气象站点分钟观测数据的容器

CLogFile logfile;	//日志类

int main(int argc, char *argv[])
{
  // inifile outpath logfile
  if (argc != 5)
  { // 如果参数非法，给出帮助文档
    printf("Using:./crtsurfdata5 infile outpath logfile datafmt\n");
    printf("Example:/root/project/idc1/bin/crtsurfdata5 /root/project/idc1/ini/stcode.ini /tmp/surfdata /log/idc/crtsurfdata5.log xml,json,csv\n\n");
    
    printf("infile   全国气象站点参数文件名。\n");
    printf("outpath  全国气象站点数据文件存放的目录。\n");
    printf("logfile  本程序运行的日志文件名。\n\n");
    printf("datafmt  生成数据文件的格式，支持xml、json、vsc三种格格式，中间用“,”分隔\n\n");

    return -1;
  }

  if (logfile.Open(argv[3], "a+", false) == false)
  {
    printf("logfile.Open(%s) failed.\n", argv[3]);
    return -1;
  }

  logfile.Write("crtsurfdata5 开始运行。\n");
  
  //把站点参数文件中加载到vstcode。
  if (LoadSTCode(argv[1]) == false) return -1;

  // 模拟生成全国气象站点分钟观测数据，存放在vsurfdata容器中
  CrtSurfData();

  // 把容器vsurfdata中的全国气象站点分钟观测数据写入文件
  if (strstr(argv[4], "xml") != 0) CtrSurfFile(argv[2], "xml");
  if (strstr(argv[4], "json") != 0) CtrSurfFile(argv[2], "json"); 
  if (strstr(argv[4], "csv") != 0) CtrSurfFile(argv[2], "csv");
  
  logfile.Write("crtsurfdata5 运行结束。\n");

  return 0;
}
bool LoadSTCode(const char *inifile)
{
  CFile File;

  // 打开站点参数文件
  if (File.Open(inifile, "r") == false) // 打开失败
  {
    logfile.Write("File.Open(%s) failed.\n", inifile);
    return false;
  }

  char strBuffer[301];
  CCmdStr CmdStr;
  struct st_code stcode;

  while (true)
  {
    // 从站点参数文件中读取一行，如果已读取完，跳出循环。
    if (File.Fgets(strBuffer, 300, true) == false) break;
       //Fgets函数用于读取一行信息
    
    //把读取到的一行进行拆分
    CmdStr.SplitToCmd(strBuffer, ",", true);
    if (CmdStr.CmdCount() != 6) continue;//站点参数文件第一行是无效数据

    //把站点参数的每个数据项保存到站点参数结构体中
    CmdStr.GetValue(0, stcode.provname, 30);
    CmdStr.GetValue(1, stcode.obtid, 10);
    CmdStr.GetValue(2, stcode.obtname, 30);
    CmdStr.GetValue(3, &stcode.lat);
    CmdStr.GetValue(4, &stcode.lon);
    CmdStr.GetValue(5, &stcode.height);

    // 把站点参数结构体放入到站点参数容器
    vstcode.push_back(stcode);
  }
  /*
  for (int ii = 0; ii < vstcode.size(); ii++)
    logfile.Write("provname=%s,obitid =%s,obtname=%s,lat=%.2f,lon=%.2f\n",
                   vstcode[ii].provname, vstcode[ii].obtid,vstcode[ii].obtname,                    vstcode[ii].lat, vstcode[ii].lon);
  这段用于测试站点参数文件是否放入站点参数容器中*/
  return true;
}
void CrtSurfData()
{
  // 播随机数种子。
  srand(time(0));

  // 获取当前时间，当成观测时间
  memset(strddatetime, 0, sizeof(strddatetime));  // strddatetime是全局变量
  LocalTime(strddatetime, "yyyymmddhh24miss");
  
  struct st_surfdata stsurfdata;

  // 遍历气象站点参数的vscode容器
  for (int ii = 0; ii < vstcode.size(); ii++)
  {
    memset(&stsurfdata, 0, sizeof(struct st_surfdata));

    // 用随机数填充分钟观测数据的结构体
    strncpy(stsurfdata.obtid, vstcode[ii].obtid, 10); // 站点代码
    strncpy(stsurfdata.ddatetime, strddatetime, 14);  // 数据时间
    stsurfdata.t = rand() % 351;          // 气温：单位0.1摄氏度
    stsurfdata.p = rand() % 265 + 10000;  // 气压：0.1百帕
    stsurfdata.u = rand() % 100 + 1;      // 相对湿度：0-100之间的值
    stsurfdata.wd = rand() % 360;         // 风向：0-360之间的值
    stsurfdata.wf = rand() % 150;         // 风速：单位0.1m/s
    stsurfdata.r = rand() % 16;           // 降雨量：0.1mm
    stsurfdata.vis = rand() % 5001 + 100000; // 能见度：0.1米
    
    // 把观测数据的结构体放入vsurfdata容器
    vsurfdata.push_back(stsurfdata);
  }
}
// 把容器vsurfdata中的全国气象站点分钟观测数据写入文件
bool CtrSurfFile(const char *outpath, const char *datafmt)
{
  CFile File;
  // 拼接生成数据的文件名，例如：/tmp/surfdata/SURF_ZH_202203091813_2254.csv
  char strFileName[301];
  sprintf(strFileName, "%s/SURF_ZH_%s_%d.%s", outpath, strddatetime, getpid(), datafmt); 
 //将后面的字符拼接到strFileName中

  // 打开文件
  if (File.OpenForRename(strFileName, "w") == false)
  {
    logfile.Write("File.OpenForRename(%s) failed.\n", strFileName);
    return false;
  }

  // 写入第一行标题。只有当写csv格式文件时需要
  if (strcmp(datafmt, "csv") == 0)
    File.Fprintf("站点代码,数据时间,气温,气压,相对湿度,风向.风速,降雨量,能见度\n");
  if (strcmp(datafmt, "xml") == 0)
    File.Fprintf("<data>\n");
  if (strcmp(datafmt, "json") == 0)
    File.Fprintf("{\"data\":[\n");

  // 遍历存放观测数据的vsurfdata容器
  for (int ii = 0; ii < vsurfdata.size(); ii++)
  {
    // 写入一条记录
    if (strcmp(datafmt, "csv") == 0)
    {
      File.Fprintf("%s,%s,%.1f,%.1f,%d,%d,%.1f,%.1f,%.1f,\n", vsurfdata[ii].obtid, vsurfdata[ii].ddatetime, vsurfdata[ii].t / 10.0, vsurfdata[ii].p / 10.0, vsurfdata[ii].u, vsurfdata[ii].wd, vsurfdata[ii].wf / 10.0, vsurfdata[ii].r / 10.0, vsurfdata[ii].vis / 10.0);
    }

    if (strcmp(datafmt, "xml") == 0)
    {
      File.Fprintf("<obtid>%s</obtid><ddatetime>%s</ddatetime><t>%.1f</t><p>%.1f</p><u>%d</u><wd>%d</wd><wf>%.1f</wf><r>%.1f</r><vis>%.1f</vis><endl/>\n", vsurfdata[ii].obtid, vsurfdata[ii].ddatetime, vsurfdata[ii].t / 10.0, vsurfdata[ii].p / 10.0, vsurfdata[ii].u, vsurfdata[ii].wd, vsurfdata[ii].wf / 10.0, vsurfdata[ii].r / 10.0, vsurfdata[ii].vis / 10.0);
    }

    if (strcmp(datafmt, "json") == 0)
    {
      File.Fprintf("{\"obtid\":\"%s\",\"ddatetime\":\"%s\",\"t\":\"%.1f\",\"p\":\"%.1f\",\"u\":\"%d\",\"wd\":\"%d\",\"wf\":\"%.1f\",\"r\":\"%.1f\",\"vis\":\"%.1f\"}", vsurfdata[ii].obtid, vsurfdata[ii].ddatetime, vsurfdata[ii].t / 10.0, vsurfdata[ii].p / 10.0, vsurfdata[ii].u, vsurfdata[ii].wd, vsurfdata[ii].wf / 10.0, vsurfdata[ii].r / 10.0, vsurfdata[ii].vis / 10.0);
    }
    if (ii < vsurfdata.size() - 1) 
      File.Fprintf(",\n");
    else 
      File.Fprintf("\n");
  }
  
  if (strcmp(datafmt, "xml") == 0)
    File.Fprintf("</data>\n");
  if (strcmp(datafmt, "json") == 0)
    File.Fprintf("]}\n");

  // 关闭文件
  File.CloseAndRename();
  
  logfile.Write("生成数据文件%s成功，数据时间%s，记录数%d。\n", strFileName, strddatetime, vsurfdata.size());
  
  return true;
}
