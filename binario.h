#ifndef BINARIO_H
#define BINARIO_H
#include <stdlib.h>
#include "auxiliar.h"

/**
 * Retorna o valor do bit na posicao indicada.
 * @param binário: o valor em binário inicial.
 * @param pos: posicao do bit no vetor de bit.
 */
ent_t getBitAt_LE(ent_t binario, int pos);
ent_t getBitAt_BE(ent_t binario, int pos);

/**
 * Coleta os bits em um range de {to} até {from}
 * @param binário: o valor em binário inicial.
 * @param to: ultimo bit da faixa de bits desejada.
 * @param from: primeiro bit da faixa de bits desejada. 
 */
ent_t getBitInRange_LE(ent_t binario, int from, int to);
ent_t getBitInRange_BE(ent_t binario, int from, int to);


ent_t turnon_BitAt(ent_t bin, int pos);
ent_t turnoff_BitAt(ent_t bin, int pos);

/**
 * Imprime na tela um valor em binário. Assumindo Little Endian.
 * @param size: sizeof(variavel que armazena o valor em binário)
 * @param ptr: endereço para a variavel que armazena o valor em binário.
 */
void printBits(size_t const size, void const * const ptr);

#endif