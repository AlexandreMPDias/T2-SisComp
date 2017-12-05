#include "error_handler.h"
#include "log.c"

#define output debug(PRE, 0,

void EH_signal(int signum, sighandler_t handler){
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
        exit(1)
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
