/*
 * Sistemas de Computação - INF1019 - 2017.2
 * Prof Markus Endler
 * Trabalho 2 - Simulando Memória Virtual e Substituição de Páginas LFU
 * 
 * Aluno1: Alexandre de Mello. P Dias - 1413183
 * Aluno2: Caio Dia de Festa - xxxxxxxx
 * 
 */

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include "error_handler.h"
#include "auxiliar.h"
#define N_PROCESS 4
#define print debug(PRE, 0,

#define PID_ 0
#define SEG_ 1


#define _left_ 0xffff0000
#define _right_ 0x0000ffff

#define _max_ 0xffffffff


//---->> Variaveis Globais

/*
 * Vetor alocado na memória compartilhada para realizar o mapeamento entre o process_id e o segmento em que a page_table desse processo está alocada.
 */
int* pair_pid[2];
int Fault_Key=7100, Pair_Key=7200;
int Process_Key;
int	process_shm[N_PROCESS]={ 8000 , 10000 , 12000 , 14000 };

//----->> estrutura com informaçoes sobre o page-fault
typedef struct fault_iformation{
	int pid;
	u_short Virtual_Page;
	
}Fault_Information;

//---->> Funções utilizadas

/*
 * create_process
 * Cria um processo, acessa a tabela desse processo no segmento especificado da memória compartilhada. Em seguida, le as instruções do arquivo recebido.
 * @param arquivo: nome do arquivo que deverá ser lido por esse processo.
 * @param shm: segmento na memória compartilhada usada para alocar a page_table deste processo.
 */
void create_process(char* arquivo,int shm);

/*
 * trans
 * --descrição e explicação desse método estão contidos no enunciado do trabalho.
 */
bool trans(pid_t, u_short, u_short, char); 

u_int get_segmento(pid_t pid);


u_int look_table(int segmento, u_short number, int side){
	int i;
	u_int entry;
	u_short ret;
	u_short s_entry;
	u_int othe_side;
	u_int* table = (u_int*)EH_shmat(segmento, 0, 0);

	if(side == _right_){
		other_side = _left_;
	}
	else{
		other_side = _right_;
	}
	for(i = 0; i < 256; i++ ){
		entry = table[i] & side;
		if(side == _left_){
			s_entry = entry >> 16;
		}
		else{
			s_entry = entry;
		}
		if(s_entry == number){
			if(side == _right_){
				return (table[i] & _left_) >> 16;
			}
			else{
				return table[i] & _right_;
			} 
		}
		
	}
	return _max_;		
}

int main(void){
	int 	i, segment;
	u_int	tables[N_PROCESS];
	pid_t	pid;
	Fault_Information information;
	char	process_names[][N_PROCESS]={ "compilador.log" , "compressor.log" , "matriz.log" , "simulador.log" };

	EH_signal( SIGUSR2, sig_handler );
	EH_signal( SIGUSR1, sig_handler);

	segment = EH_shmget(Fault_Key, sizeof(Fault_Information), IPC_CREAT | S_IRUSR | S_IWUSR);
	for( i = 0 ; i < N_PROCESS; i++ ){
		pid = EH_fork();
		if( pid == 0 ){
			process_Key=process_shm[i];
			segment = EH_shmget(process_shm[i], 256 * sizeof(u_int), IPC_CREAT | S_IRUSR | S_IWUSR);
			create_process(process_names[i]);
		}
	}
	while(true){
		//pegar numero o tempo corrente e checar se ja passou 30ms
		//se passou zerar o cache
	}
}

void create_process(char* arquivo){
	un_int		addr;
	u_short		i , o;
	pid_t		pid;
	char		rw;
	FILE*		file;

	file = EH_fopen(arquivo,"r");
	pid = getpid();
	while(fscanf(file,"%x %c ", &addr, &rw)){
		i = (u_short)((addr & _left_) >> 16);
		o = (u_short)(addr & _right_);
		trans(pid,i,o,rw);
	}
	fclose(file);
}

void sig_handler(int signal){
	int segment;
	int pid;
	u_short virtual_page;
	u_int *Table;
	Fault_Information information;
	if(signal == SIGUSR1){
		segment = EH_shmget(Fault_Key, sizeof(Fault_Information),  S_IRUSR | S_IWUSR);
		information = (Fault_Information)EH_shmat(segment, 0, 0);
		segment = EH_shmget(FP, 256 * sizeof(u_int),  S_IRUSR | S_IWUSR);
		Table = (u_int *)EH_shmat(segment, 0, 0);
		pid=information.pid;
		virtual_page=information.Virtual_Page;
		//achar um frame livre pra esse par processo-pagina virtual
	}
	else{
		printf ("SIGNAL: %d\n", signal);
	}
}

bool trans(pid_t pid, u_short i, u_short offset, char rw){
	//abrir mem. compartilhada do processo
	//percorrer table de memoria compartilhada checando se possui mapeamento
	int segmento = get_segmento(pid);
	if(true) {//se sim, imprime
		printf("%d, %04x, %04x, %c\n", pid, (u_short)i,offset,rw);
		return true;
	}
	else{ 	//se nao, avisa o GM que houve pagefault
		//salva o numero do processo requerinte e pagina virtual nao mapeada em uma outra memoria compartilhada(precisa ser criada pelo processo pai)
		kill(ppid(), SIGUSR1);
		raise(SIGSTOP);
		sleep(1);
		return false;
	}
}

u_int get_segmento(pid_t pid){
	int i;
	for(i=0; i < N_PROCESS; i++){
		if(pair_pid[i][PID_] = pid){
			return pair_pid[i][SEG_];
		}
	}
	return -1;
}
