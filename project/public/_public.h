#ifndef __PUBLIC_HH
#define __PUBLIC_HH 1

#include "_cmpublic.h"

///////////////////////////////////// /////////////////////////////////////
// 以下是字符串操作相关的函数和类

// 安全的strcpy函数。
// dest：目标字符串，不需要初始化，在STRCPY函数中会对它进行初始化。
// destlen：目标字符串dest占用内存的大小。
// src：原字符串。
// 返回值：目标字符串dest的地址。
// 注意，超出dest容量的内容将丢弃。
char *STRCPY(char* dest,const size_t destlen,const char* src);

// 安全的strncpy函数。
// dest：目标字符串，不需要初始化，在STRNCPY函数中会对它进行初始化。
// destlen：目标字符串dest占用内存的大小。
// src：原字符串。
// n：待复制的字节数。
// 返回值：目标字符串dest的地址。
// 注意，超出dest容量的内容将丢弃。
char *STRNCPY(char* dest,const size_t destlen,const char* src,size_t n);

// 安全的strcat函数。
// dest：目标字符串。
// destlen：目标字符串dest占用内存的大小。
// src：待追加的字符串。
// 返回值：目标字符串dest的地址。
// 注意，超出dest容量的内容将丢弃。
char *STRCAT(char* dest,const size_t destlen,const char* src);

// 安全的strncat函数。
// dest：目标字符串。
// destlen：目标字符串dest占用内存的大小。
