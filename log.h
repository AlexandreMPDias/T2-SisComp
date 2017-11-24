#ifndef MY_LOG_H
#define MY_LOG_H
#include <stdarg.h>

/**
 * Esse é o modulo Log
 * 
 * A unica funcao desse modulo é a funçao debug, e para chama-la, basta chamanda da seguinte forma:
 * debug(PRE, debugLevel, fmt, ...)
 * 
 * O debug_level é crescente, para funcoes raizes, como main, escalonadores, etc... Deve-se usar um debug de menor valor.
 * Para comentarios mais especificos, mais enraizado dentro do corpo do programa, Deve-se usar um debug de maior valor.
 * 
 * Para alterar entre imprimir e nao imprimir, basta comentar __PRINT__
 * 
 */

#define __PRINT__
#ifdef __PRINT__
static unsigned int __debugLevel__ = 0; 
#else
static unsigned int __debugLevel__ = 10000; 
#endif

#define PRE __FILE__,__func__


void debug(const char* file, const char* func,  unsigned  int debugLevel, const char *fmt, ...);


#endif