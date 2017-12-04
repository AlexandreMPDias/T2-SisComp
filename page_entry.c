#include "page_entry.h"
#include "binario.h"

#define PROTECTION_LENGTH 4

int extrair(int informacao, ent_t entrada_tabela){
    int bit_start, bit_end;
    int bit_size = sizeof(ent_t);
    int page_length = (bit_size - (4 + PROTECTION_LENGTH));
    switch(informacao){
        case CACHING: {
            bit_start = bit_size - 1;
            bit_end = bit_start;
            break;
        }
        case REFERENCED: {
            bit_start = bit_size - 2;
            bit_end = bit_start;
            break;
        }
        case MODIFIED: {
            bit_start = bit_size - 3;
            bit_end = bit_start;
            break;
        }
        case PROTECTION: {
            bit_start = bit_size - 4;
            bit_end = bit_size - (4 + PROTECTION_LENGTH);
            break;
        }
        case PRESENT: {
            bit_start = bit_size - (4 + PROTECTION_LENGTH) - 1;
            bit_end = bit_start;
            break;
        }
        case PAGE_FRAME_NUMBER: {
            bit_start = 0;
            bit_end = page_length;
            break;
        }
        default:{
            printf("Informacao Invalida. [%d]\n", informacao);
            return -1;
        }
    }

    if(bit_start == bit_end){
        return getBitAt_LE(entrada_tabela, bit_start);
    }
    else{
        return getBitInRange_LE(entrada_tabela, bit_start, bit_end);
    }
}

ent_t criar_entrada(unsigned int page_frame_number, int esta_na_ram, int controle_de_acesso){
    long frame_mask = 4 + PROTECTION_LENGTH + 5;
    ent_t bitvec = (page_frame_number << frame_mask) >> frame_mask;
}

void foi_referenciada(ent_t* entrada_tabela);

void foi_modificada(ent_t* entrada_tabela);

void referencia_expirada(ent_t* entrada_tabela);

void modificacao_expirada(ent_t* entrada_tabela);

void virtual_address(v_addr addr, int *page_number, int *offset){
    *page_number = addr >> 11;
    *offset = 0x111 & addr;
}
void physical_address(f_addr addr, int *page_number, int *offset){
    *page_number = addr & 0x0111 1000;
    *offset = 0x111 & addr;
}
