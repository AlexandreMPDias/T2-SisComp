#ifndef BINARIO_H
#define BINARIO_H
#include <stdlib.h>

/**
 * Retorna o valor do bit na posicao indicada.
 * @param binário: o valor em binário inicial.
 * @param pos: posicao do bit no vetor de bit.
 */
int getBitAt_LE(int binario, int pos);
int getBitAt_BE(int binario, int pos);

/**
 * Coleta os bits em um range de {to} até {from}
 * @param binário: o valor em binário inicial.
 * @param to: ultimo bit da faixa de bits desejada.
 * @param from: primeiro bit da faixa de bits desejada. 
 */
int getBitInRange_LE(int binario, int from, int to);
int getBitInRange_BE(int binario, int from, int to);

/**
 * Imprime na tela um valor em binário. Assumindo Little Endian.
 * @param size: sizeof(variavel que armazena o valor em binário)
 * @param ptr: endereço para a variavel que armazena o valor em binário.
 */
void printBits(size_t const size, void const * const ptr);

#endif