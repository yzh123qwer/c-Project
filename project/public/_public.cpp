#include "_public.h"

// 安全的strcpy函数。
// dest：目标字符串，不需要初始化，在STRCPY函数中有初始化代码。
// destlen：目标字符串dest占用内存的大小。
// src：原字符串。
// 返回值：目标字符串dest的地址。
// 注意，超出dest容量的内容将丢弃。
char *STRCPY(char* dest,const size_t destlen,const char* src)
{
  if (dest==0) return 0;    // 判断空指针。
  memset(dest,0,destlen);   // 初始化dest。
  // memset(dest,0,sizeof(dest));   // 不能这么写，在64位系统中，sizeof(dest)永远是8。
  if (src==0) return dest;

  if (strlen(src)>destlen-1) strncpy(dest,src,destlen-1);
  else strcpy(dest,src);

  return dest;
}

// 安全的strncpy函数。
// dest：目标字符串，不需要初始化，在STRCPY函数中有初始化代码。
// destlen：目标字符串dest占用内存的大小。
// src：原字符串。
// n：待复制的字节数。
// 返回值：目标字符串dest的地址。
// 注意，超出dest容量的内容将丢弃。
char *STRNCPY(char* dest,const size_t destlen,const char* src,size_t n)
{
  if (dest==0) return 0;    // 判断空指针。
  memset(dest,0,destlen);   // 初始化dest。
  // memset(dest,0,sizeof(dest));   // 不能这么写，在64位系统中，sizeof(dest)永远是8。
  if (src==0) return dest;

  if (n>destlen-1) strncpy(dest,src,destlen-1);
