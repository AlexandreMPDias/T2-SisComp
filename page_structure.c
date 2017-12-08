#include "page_structure.h"

#define FREQUENCY 0
#define ADDR 1
#define PAIR 2

#define debug_level_page_structure 3
#define logging debug(PRE, debug_level_page_structure,
#define CACHE_SIZE 5

#define INV_ADDR -1

un_int freq[CACHE_SIZE][3];
bool created = false;

int find_least_freq();
bool set_freq(int i, un_int addr, un_int f);

void fill_structure(un_int structure[], un_int start){
    int i;
    for(i = 0; i < PAGE_FRAME_SIZE; i++){
        structure[i] = start + i;
    }
}

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

bool is_inside_cache(un_int addr){
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

un_int access_addr(un_int addr){
    int i, index_desejado, cache_size = 0;
    for( i = 0, index_desejado = -1; i < CACHE_SIZE; i++){
        if ( freq[i][ADDR] == addr ) {
            index_desejado = i;
            break;
        }
    }
    for(i = 0; i < CACHE_SIZE; i++){
        if ( freq[i][ADDR] != 0 ) {
            cache_size++;
        }
    }
    if(cache_size == CACHE_SIZE){
        if(is_inside_cache(addr) == false && cache_size == CACHE_SIZE){
            logging "Cache cheio. Endereco Novo. Trocando Entradas.\n");
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
            logging "Cache cheio. Endereco Repetido. Atualizando Frequencia.\n");
            freq[index_desejado][FREQUENCY]++;
            return freq[index_desejado][PAIR];
        }
    }
    else{
        logging "Inserindo Nova Entrada no Cache. Preenchimento = [%d/%d]\n",cache_size+1, CACHE_SIZE);
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
    for(i=0; i < CACHE_SIZE; i++){
        if(freq[i][FREQUENCY] < sf){
            sf = freq[i][FREQUENCY];
            si = i;
        }
    }
    if(i == -1){
        logging "Erro.\n");
    }
    return i;
}

bool set_freq(int i, un_int addr, un_int frequency){
    freq[i][ADDR] = addr;
    freq[i][FREQUENCY] = frequency;
    freq[i][PAIR] = addr;
    return true;
}
bool add_freq(int i, un_int addr){
    freq[i][ADDR] = addr;
    freq[i][FREQUENCY] += 1;
    freq[i][PAIR] = addr;
    return true;
}

void clear_cache(){
    int i;
    for(i = 0; i < CACHE_SIZE; i++){
        freq[i][FREQUENCY] = 0;
        freq[i][ADDR] = 0;
        freq[i][PAIR] = 0;
    }
    logging "Cache resetado.\n");
}

bool reset_frequency(un_int addr){
    int i;
    for(i = 0; i < CACHE_SIZE; i++){
        if ( freq[i][ADDR] == addr) {
            freq[i][FREQUENCY] = 1;
            logging "Frequencia resetada.\n");
        }
    }
}