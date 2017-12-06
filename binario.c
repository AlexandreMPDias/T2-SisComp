#include "binario.h"
#include <stdlib.h>
#include <stdio.h>

#define BIN_MASK 0x01

void checkNegative(ent_t value);

ent_t getBitAt_LE(ent_t binario, int pos){
    checkNegative(pos);
    return ((binario >> pos) & BIN_MASK);
}

ent_t getBitAt_BE(ent_t binario, int pos){
    checkNegative(pos);
    return getBitAt_LE(binario, 32-pos);
}

ent_t getBitInRange_LE(ent_t binario, int from, int to){
    ent_t val = binario;
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

ent_t getBitInRange_BE(ent_t binario, int from, int to){
    getBitInRange_LE(binario, 32 - from, 32 - to);
}

void checkNegative(ent_t value){
    if(value < 0){
        printf("Binario: Recebendo valor de posicao negativa.\nEncerrando execucao.\n");
        exit(1);
    }
}


void printBits(size_t const size, void const * const ptr)
{
    unsigned char *b = (unsigned char*) ptr;
    unsigned char byte;
    size_t i;
    int j;

    for (i=size-1;i>=0;i--)
    {
        for (j=7;j>=0;j--)
        {
            byte = (b[i] >> j) & 1;
            printf("%u", byte);
        }
    }
    puts("");
}

ent_t switch_BitAt(ent_t bin, int pos, bool value){
    ent_t newbin = 1L << pos;
    if(value == 1){
        return bin | newbin;
    }
    else{
        return bin & newbin;
    }

}

ent_t turnon_BitAt(ent_t bin, int pos){
    return switch_BitAt(bin, pos, true);
}

ent_t turnoff_BitAt(ent_t bin, int pos){
    return switch_BitAt(bin, pos, false);
}