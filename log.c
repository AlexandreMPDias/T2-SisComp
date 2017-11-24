#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void tabs(int length);
void print_by_letter(const char* s);

void format(const char* s, int debugLevel){
    tabs(debugLevel);
    printf("[ ");
    print_by_letter(s);
    printf(" ]: ");
}

void format2(const char* s, const char* v, int debugLevel){
    tabs(debugLevel);
    printf("[ ");
    print_by_letter(s);
    printf(" > ");
    print_by_letter(v);
    printf(" ]: ");
}

int is_empty(const char* s){
    return (s == NULL || strlen(s) == 0);
}

void tabs(int n){
    int i;
    for(i=0; i < n; i++) printf("%c", '\t');
}

void print_by_letter(const char* s){
    int i = 0;
    int size = strlen(s);
    if(s[0] >= 97 && s[0] <= 122){
        i = 32;
    }
    printf("%c", s[0] - (char)i);
    for(i = 1; i < size && s[i] != '\0' && s[i] != '.'; i++){
       printf("%c",s[i]);
    }
}

void debug(const char* file, const char* function, unsigned int debugLevel,  const char* fmt, ...){
#ifndef PRINT_NOTHING
    if(debugLevel < __debugLevel__) return;
    if(is_empty(file) && !is_empty(function)){
        format(function,debugLevel);
    }
    else if(!is_empty(file) && is_empty(function)){
        format(file,debugLevel);
    }
    else {
        format2(file,function,debugLevel);
    }
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
#endif
}
