#ifndef PAGE_ENTRY_H
#define PAGE_ENTRY_H
#include "auxiliar.h"


#define true 1
#define false 0
#define bool int

#define CACHING 0
#define REFERENCED 1
#define MODIFIED 2
#define PROTECTION 3
#define PRESENT 4
#define PAGE_FRAME_NUMBER 5

int extrair(int informacao, ent_t entrada_tabela);

ent_t criar_entrada(int page_frame_number, int esta_na_ram, int controle_de_acesso);

void foi_referenciada(ent_t* entrada_tabela);

void foi_modificada(ent_t* entrada_tabela);

void referencia_expirada(ent_t* entrada_tabela);

void modificacao_expirada(ent_t* entrada_tabela);


#endif