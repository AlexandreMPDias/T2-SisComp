#include <stdlib.h>
#include <stdio.h>

#define _left_			0xffff0000
#define _right_			0x0000ffff

int main(){
    printf("%x\n%x\n", _left_, ~_right_);
}