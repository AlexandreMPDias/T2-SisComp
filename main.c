#include <stdio.h>
#include "binario.h"
#include "log.h"

void foo(){
    debug(PRE, 1, "Iniciando!\n");
    debug(PRE, 1, "Finalizando!\n");
}

int main(){
    long i = 123L;
    char *s = "Hola amigos";
    debug(PRE, 0, "Welldone.\n");
    foo();
    debug(PRE, 1, "O valor e %ld -- %s\n", i,s);
}