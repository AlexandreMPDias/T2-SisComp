#ifndef PAGE_STRUCTURE_H
#define PAGE_STRUCTURE_H

#include "auxiliar.h"

#define PAGE_FRAME_SIZE 64 * 1024 * 8 // 524288bits // 64Kb

typedef un_int* Frame;
typedef un_int* Page;

void fill_structure(un_int structure[], un_int start);

void create_cache();
void clear_cache();
bool is_inside_cache(un_int addr);
un_int access_addr(un_int addr);

#endif
