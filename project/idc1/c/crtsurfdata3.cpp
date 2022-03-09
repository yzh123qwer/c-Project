/* 
 * 程序名：crtsurfdata3.cpp 本程序用于生成全国气象站点每分钟生成的数据
 * 作者：姚子涵
*/
#include"_public.h"

bool LoadSTCode(const char *inifile); 
// 把站点参数文件参数加载到vstcode容器中

void CrtSurfData();
// 模拟生成气象观测数据，存放在vsurfdata容器中。

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
  if (argc != 4)
  { // 如果参数非法，给出帮助文档
    printf("Using:./crtsurfdata3 infile outpath logfile\n");
    printf("Example:/root/project/idc1/bin/crtsurfdata3 /root/project/idc1/ini/stcode.ini /tmp/surfdata /log/idc/crtsurfdata3.log\n\n");
    
    printf("infile   全国气象站点参数文件名。\n");
    printf("outpath  全国气象站点数据文件存放的目录。\n");
    printf("logfile  本程序运行的日志文件名。\n\n");

    return -1;
  }

  if (logfile.Open(argv[3], "a+", false) == false)
  {
    printf("logfile.Open(%s) failed.\n", argv[3]);
    return -1;
  }

  logfile.Write("crtsurfdata3 开始运行。\n");
  
  //把站点参数文件中加载到vstcode。
  if (LoadSTCode(argv[1]) == false) return -1;

  // 模拟生成全国气象站点分钟观测数据，存放在vsurfdata容器中
  CrtSurfData();

  logfile.Write("crtsurfdata3 运行结束。\n");

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
  char strddatetime[21];
  memset(strddatetime, 0, sizeof(strddatetime));
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
