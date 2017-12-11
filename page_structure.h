#ifndef PAGE_STRUCTURE_H
#define PAGE_STRUCTURE_H

typedef unsigned int u_int;

#include "auxiliar.h"

#define PAGE_FRAME_SIZE 64 * 1024 * 8 // 524288bits // 64Kb

/**
 * create_cache
 * Inicializa o cache.
 */
void create_cache();

/**
 * clear_cache
 * Limpa o Cache, resetando a frenquencia de acesso de todos os endereços para zero.
 */
void clear_cache();

/**
 * is_inside_cache
 * Checa se o endereço recebido se encontra dentro do Cache.
 */
bool is_inside_cache(u_int addr);

/**
 * access_addr
 * Acessa um endereço, atualizando sua frequencia dentro do Cache.
 */
u_int access_addr(u_int addr);


/**
 * get_least_frequency
 * 
 * Itera dentro do Cache e retorna o endereço com menor frequencia de acesso.
 */
 u_int get_least_frequency();

#endif
