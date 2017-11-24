#ifndef MY_LOG_H
#define MY_LOG_H
#include <stdarg.h>

//#define PRINT_NOTHING
#define PRINT_ALL
#ifdef PRINT_ALL
static unsigned int __debugLevel__ = 0; 
#else
static unsigned int __debugLevel__ = 1; 
#endif

#define PRE __FILE__,__func__

void debug(const char* file, const char* func,  unsigned  int debugLevel, const char *fmt, ...);


#endif