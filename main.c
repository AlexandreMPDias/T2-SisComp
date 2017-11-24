#include <stdio.h>
#include "binario.h"
#include "log.h"

void foo(){
    debug(PRE, 1, "Iniciando!\n");
    debug(PRE, 1, "Finalizando!\n");
}

int main(){
    debug(PRE, 0, "Welldone.\n");
    foo();
}