#include "binario.h"
#include <stdlib.h>
#include <stdio.h>

#define BIN_MASK 0x01

void checkNegative(int value);

int getBitAt_LE(int binario, int pos){
    checkNegative(pos);
    return ((binario >> pos) & BIN_MASK);
}

int getBitAt_BE(int binario, int pos){
    checkNegative(pos);
    return getBitAt_LE(binario, 32-pos);
}

int getBitInRange_LE(int binario, int from, int to){
    unsigned int val = binario;
    checkNegative(from);
    checkNegative(to);
    int t,f;
    if(to > from){
        t = to;
        f = from;
    }
    else if(to < from){
        t = from;
        f = to;
    }
    else{
        printf("Range do Bit igual a 0.\nEncerrando a execucao.\n");
        exit(1);
    }
    val = (((val << to) >> to) >> from);
    return val;
}

int getBitInRange_BE(int binario, int from, int to){
    getBitInRange_LE(binario, 32 - from, 32 - to);
}

void checkNegative(int value){
    if(value < 0){
        printf("Binario: Recebendo valor de posicao negativa.\nEncerrando execucao.\n");
        exit(1);
    }
}