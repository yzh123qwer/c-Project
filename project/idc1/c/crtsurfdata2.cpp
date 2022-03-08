/*
 * 程序名：crtsurfdata2.cpp 本程序用于生成全国气象站点每分钟生成的数据
 * 作者：姚子涵
*/
#include"_public.h"
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

//把站点参数文件中参数加载到vstcode容器中
bool LoadSTCode(const char *inifile);

CLogFile logfile;       //日志类
int main(int argc, char *argv[])
{
  // inifile outpath logfile
  if (argc != 4)
  {
    printf("Using:./crtsurfdatal infile outpath logfile\n");

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

  logfile.Write("crtsurfdata2 开始运行。\n");

  //把站点参数文件中加载到vstcode。
  if (LoadSTCode(argv[1]) == false) return -1;

  logfile.Write("crtsurfdata2 运行结束。\n");

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
