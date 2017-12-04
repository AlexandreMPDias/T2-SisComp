#include "page_structure.h"
#define logging debug(PRE, 2,

int main(){
    un_int a[PAGE_FRAME_SIZE];
    int i;
    create_cache();
    un_int b[8] = {1,2,3,4,5,6,7,8};
    for(i = 0; i < 8; i++){
        logging "%d - %d\n", b[i], access_addr(b[i]));
    }
    for(i = 0; i < 3; i++){
        logging "%d - %d\n", b[i], access_addr(b[i]));
    }
    for(i=0; i < 3; i++){
        logging "%d - %d\n", b[2], access_addr(b[2]));
    }
}
