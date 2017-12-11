#include "page_structure.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/shm.h>

#define FREQUENCY 0
#define ADDR 1
#define PAIR 2

#define debug_level_page_structure 1
#define logging debug(PRE, debug_level_page_structure,
#define CACHE_SIZE 256

#define INV_ADDR -1

u_int freq[CACHE_SIZE][3];
bool created = false;

int find_least_freq();
bool set_freq(int i, u_int addr, u_int f);

void create_cache(){
    int i;
    if(created == false){
        created = true;
        for(i = 0; i < CACHE_SIZE; i++){
            freq[i][FREQUENCY] = 0;
            freq[i][ADDR] = 0;
            freq[i][PAIR] = 0;
        }
        logging "Cache criado com tamanho [ %d ].\n", CACHE_SIZE);
    }
    else{
        logging "Atencao: Cache ja criado.\n");
    }
}

bool is_inside_cache(u_int addr){
    int i;
    for(i = 0; i < CACHE_SIZE; i++){
        if(freq[i][ADDR] == addr){
            logging "Endereco dentro do Cache.\n");
            return true;
        }
    }
    logging "Endereco fora do Cache.\n");
    return false;
}

u_int access_addr(u_int addr){
    int i, index_desejado, cache_size = 0;
    for( i = 0, index_desejado = -1; i < CACHE_SIZE; i++){
        if ( freq[i][ADDR] == addr ) {
            index_desejado = i;
        }
        if ( freq[i][ADDR] != 0 ) {
            cache_size++;
        }
    }
    logging "Acessando %x\n", addr);
    if(cache_size == CACHE_SIZE){
        if(is_inside_cache(addr) == false && cache_size == CACHE_SIZE){
            logging "Endereco Novo: Trocando Entradas.\n");
            index_desejado = find_least_freq();
            if(set_freq(index_desejado, addr, 1) == false){
                logging "Erro mapeando endereco.\n");
                return INV_ADDR;
            }
            else{
                return freq[index_desejado][PAIR];
            }
        }
        else if(is_inside_cache(addr) == true && cache_size == CACHE_SIZE){
            logging "Endereco Repetido: Atualizando Frequencia.\n");
            freq[index_desejado][FREQUENCY]++;
            return freq[index_desejado][PAIR];
        }
    }
    else{
        logging "Endereco Novo: Preenchimento = [%d/%d]\n",cache_size+1, CACHE_SIZE);
        if(set_freq(cache_size, addr, 1) == false){
            logging "Erro mapeando endereco.\n");
            return INV_ADDR;
        }
        cache_size++;
        return freq[cache_size - 1][PAIR];
    }
}


int find_least_freq(){
    int i;
    int sf, si;
    sf = 0xffffffff;
    si = -1;
    for(i= 0; i < CACHE_SIZE; i++){
        if(freq[i][FREQUENCY] < sf){
            sf = freq[i][FREQUENCY];
            si = i;
        }
    }
    if(si == -1){
        logging "Erro.\n\n\n\n");
    }
    return si;
}

bool set_freq(int i, u_int addr, u_int frequency){
    freq[i][ADDR] = addr;
    freq[i][FREQUENCY] = frequency;
    freq[i][PAIR] = addr;
    return true;
}
bool add_freq(int i, u_int addr){
    freq[i][ADDR] = addr;
    freq[i][FREQUENCY] += 1;
    freq[i][PAIR] = addr;
    return true;
}

void clear_cache(){
    int i;
    for(i = 0; i < CACHE_SIZE; i++){
        //freq[i][FREQUENCY] = 0;
    }
    logging "Cache resetado.\n");
}

bool reset_frequency(u_int addr){
    int i;
    for(i = 0; i < CACHE_SIZE; i++){
        if ( freq[i][ADDR] == addr) {
            freq[i][FREQUENCY] = 1;
            logging "Frequencia resetada.\n");
        }
    }
}

u_int get_least_frequency(){
    int i = find_least_freq();
    return freq[i][ADDR];
}