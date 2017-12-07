#include "error_handler.h"
#include "log.c"

#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/mman.h>

#define output debug(PRE, 0,

void EH_signal(int signum, sighandler__t handler){
    if( signal( signum, handler ) == SIG_ERR )
	{
		output "Erro: Nao foi possivel instalar rotina de atendimento.\n");
		exit( 1 );
	}
}

pid_t EH_fork(){
    pid_t p = fork();
    if(p < 0){
        output "Erro: Nao foi possivel criar novo processo.");
        exit(1);
    }
    return p;
}

FILE* EH_fopen(const char* path, const char *desc){
    FILE* a = fopen(path, desc);
    if(a == NULL){
        output "Erro: Nao foi possivel abrir arquivo: [%s]\n" , path);
        exit( 1 );
    }
    return a;
}

int EH_shmget(key_t key,size_t size, int flags){
	int shm = shmget( key, size, flags);
	if( shm == -1 )
	{
		output "Erro: Nao foi possivel criar segmento para memoria compartilhada\n");
		exit( 1 );
	}
	return shm;
}

void* EH_shmat(int shmid, const void *shmaddr, int shmflg){
	void* a = shmat(shmid, shmaddr, shmflg);
	if( a == NULL )
	{
		output "Erro: Nao foi possivel alocar memoria compartilhada\n");
		exit( 1 );
	}
	return a;
}

