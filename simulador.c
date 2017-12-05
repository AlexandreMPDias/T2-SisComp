#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include "error_handler.h"
#include "auxiliar.h"
#define NUMBEROFPROCESS 4
char **process_names={"compilador.log","compressor.log","matriz.log","simulador.log"};
int *process_shm={8000,8200,8400,8600};
void create_process(char* arquivo,int shm);

#define print debug(PRE, 0,

int main(void){
	int i;
	EH_signal( SIGUSR2, sig_handler );
	EH_signal( SIGUSR1, sig_handler);
	pid_t pid  = EH_fork();
	for( i = 0 ; i < NUMBEROFPROCESS; i++ ){
		if( pid == 0 ){
			create_process(process_names[i]);
		}
	}
	/*Implementar gerente de memoria*/
	//criar mmap
	//criar swap
}

void create_process(char* arquivo,int shm_addr){
	un_int addr;
	u_short i , o;
	char rw;
	FILE *file ;
	file = EH_fopen(arquivo,"r");
	//criar memoria compartilhada com a page_table
	/*
	shm = shmget( shm_addr, ?????????, IPC_CREAT | S_IRUSR | S_IWUSR);
	if( shm == -1 )
	{
		printf("esc.c Erro: Nao foi possivel alocar memoria compartilhada\n");
		exit( 1 );
	}

    p  = ( char * ) shmat( shm, 0, 0 );
	if( p == NULL )
	{
		printf("esc.c: Nao foi possivel alocar memoria compartilhada\n");
		exit( 1 );
	}*/
	while(fscanf(file,"%x %c ", &addr, &rw)){
		i = addr & 0xffff;
		o = addr >> 16;
		trans(getpid(),i,o,rw);
	}
	fclose(file);
}
void sig_handler(int signal){
	switch(signal) {
		case SIGUSR1 :
		//tratar
            break;
        case SIGUSR2 :
        //tratar
            break;
        default :
            print "SIGNAL %d\n", signal);
    }
}
