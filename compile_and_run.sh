#!/bin/sh
#gcc -o main simulador.c binario.c log.c page_entry.c page_structure.c
gcc -o main simulador.c error_handler.c
./main
