#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#define NUMBEROFPROCESS 4
char **process_names={"compilador.c","compressor.c","matriz.c","simulador.c"};
int *process_shm={8000,8200,8400,8600};
void create_process(char* arquivo,int shm);
int main(void){
	int i;
	if( signal( SIGUSR2, sig_handler ) == SIG_ERR )
	{
		printf( "esc.c Erro: Nao foi possivel instalar rotina de atendimento.\n" );
		exit( 1 );
	}
	if( signal( SIGUSR1, sig_handler) == SIG_ERR )
	{
		printf( "esc.c Erro: Nao foi possivel instalar rotina de atendimento.\n" );
		exit( 1 );
	}
	pid_t pid  = fork( );
	for(i=0;i<NUMBEROFPROCESS;i++){
		if( pid < 0 ){
			printf( "simulador.c Erro: Nao foi possivel criar novo processo.\n" );
			exit( 2 );
		}
		else if( pid ==0 ){
			create_process(process_names[i]);
		}
	}
	/*Implementar gerente de memoria*/
	//criar mmap
	//criar swap

}
void create_process(char* arquivo,int shm_addr){
	unsigned int addr;
	unsigned short i , o;
	char rw;
	FILE *file ;
	file= fopen("arquivo","r");
	if(file==NULL){
		printf("falha ao abrir arquivo");
		exit(1);
	}
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
	while(fscanf(file," %x %c", &addr, &rw)){
		i=addr;
		o=addr>>16;
		trans(getpid(),i,o,rw);
		fclose(file);
	}
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
            printf("SIGNAL %d\n", signal);
    }
}