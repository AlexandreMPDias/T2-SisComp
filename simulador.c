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
#include <time.h>

#define print debug(PRE, 0,

#define PID_ 0
#define SEG_ 1
#define CPU_TIME 30

#define _n_of_process_	0x4	//4
#define _max_pages_		0x100	//256

#define _left_			0xffff0000
#define _right_			0x0000ffff
#define _max_			0xffffffff

//----->> estrutura com informaçoes sobre o page-fault
typedef struct Fault_Info__{
	int pid;
	u_short virtual_page;
}Fault_Info;

//---->> Variaveis Globais

/*
 * Vetor alocado na memória compartilhada para realizar o mapeamento entre o process_id e o segmento em que a page_table desse processo está alocada.
 */
int* pair_pid[2];


int fault_key=7100, pair_key=7200;
int process_key;
int	process_shm[_n_of_process_] = { 8000 , 10000 , 12000 , 14000 };
Fault_Info* shd_info;

//---->> Funções utilizadas

/*
 * create_process
 * Cria um processo, acessa a tabela desse processo no segmento especificado da memória compartilhada. Em seguida, le as instruções do arquivo recebido.
 * @param arquivo: nome do arquivo que deverá ser lido por esse processo.
 */
void create_process(char* arquivo, u_int sleeper);

/*
 * trans
 * --descrição e explicação desse método estão contidos no enunciado do trabalho.
 */
bool trans(pid_t, u_short, u_short, char); 

/**
 * get_segmento
 * Percorre a page_table to processo e procu
 */
u_int get_segmento(pid_t pid);

/**
 * look_table
 * Percorre a page_table to processo e procurando por um determinado valor de 16 bits dentro do endereco na tabela.
 * @param segmento: segmento para recuperar a tabela.
 * @param number: numero de 16 bits que espera-se ser encontrado dentro dos valores de 32 bits para cada entrada da page_table.
 * @param side: se side for _left_ procura o number nos 16 bits da esquerda da entrada, e se side for _right_ procura o number nos 16 bits da direita da entrada.
 * 
 * @return: uma vez encontrado o valor na tabela, retorna os 16 bits do outro lado do numero. Retorna _max_ caso o valor não seja encontrado.
 * Ex: tabela possui >somente< a entrada 87621234
 * look_table(seg, 8762, _left_) retorna 1234
 * look_table(seg, 1234, _right_) retorna 8762
 * look_table(seg, 1234, _left_) retorna _max_
 * look_table(seg, 8762, _right_) retorna _max_
 */
u_int look_table(int segmento, u_short number, int side);

/**
 * to_side
 * Corta ao meio um vetor de bits de 32 bits, e retorna uma das partes em um vetor de bits de tamanho igual a 16.
 * @param valor de 32bits que será cortado ao meio
 * @param lado do corte. Se corte = _right_ , retorna o lado direito do vetor de bits. Se corte = _left_, o mesmo acontecerá par ao lado esquerdo do vetor de bits.
 * Ex:
 * to_side(0x12345678, _left_)  -> retornará 0x1234
 * to_side(0x12345678, _right_) -> retornará 0x5678
 * 
 */
u_short to_side(u_int valor, int side);

void lock_info(pid_t,u_short);
void unlock_info();

void sleep_nano(long nanoseconds);
void sleep_ms(long ms);

int main(void){
	int 		i, segment, s;
	u_int		tables[_n_of_process];
	pid_t		pid;
	Fault_Info	information;
	char		process_names[][_n_of_process]={ "compilador.log" , "compressor.log" , "matriz.log" , "simulador.log" };

	EH_signal( SIGUSR2, sig_handler );
	EH_signal( SIGUSR1, sig_handler);
	struct timeval start_tv,corr_tv;


	segment = EH_shmget(fault_key, sizeof(Fault_Info), IPC_CREAT | S_IRUSR | S_IWUSR);
	
	shd_info = EH_shmat(segment,NULL,0);

	for( i = 0 ; i < _n_of_process; i++ ){
		pid = EH_fork();
		if( pid == 0 ){
			process_key=process_shm[i];
			segment = EH_shmget(process_shm[i], _max_pages_ * sizeof(u_int), IPC_CREAT | S_IRUSR | S_IWUSR);
			create_process(process_names[i], i);
		}
	}
	gettimeofday (&inicio_tv, NULL);
	while(true){
		gettimeofday (&corr_tv, NULL);
		if(((corr_tv.tv_usec-inicio_tv.tv_usec)>CPU_TIME)){
			inicio_tv = corr_tv;
			clear_cache();
		}
		else{
			sleep_ms(30);
		}
	}
}

void create_process(char* arquivo, u_int sleeper){
	u_int		addr;
	u_short		i , o;
	u_int*		page_table;
	pid_t		pid;
	char		rw;
	FILE*		file;

	file = EH_fopen(arquivo,"r");
	pid = getpid();

	while(fscanf(file,"%x %c ", &addr, &rw) != 0){
		i = to_side(addr, _left_);
		o = to_side(addr, _right_);
		if(trans(pid,i,o,rw)){
			/**
			 * 
			 * 
			 * 
			 * 
			 * 
			 */
		}
	}
	fclose(file);
}

void sig_handler(int signal){
	int			seg1, seg2;
	pid_t		pid;
	u_short		vt_page;
	u_int		*table;
	Fault_Info information;
	if(signal == SIGUSR1){
		seg1		= EH_shmget(fault_key, sizeof(Fault_Info),  S_IRUSR | S_IWUSR);
		information	= (Fault_Info)EH_shmat(seg1, 0, 0);
		seg2		= EH_shmget(FP, _max_pages_ * sizeof(u_int),  S_IRUSR | S_IWUSR);
		table 		= (u_int *)EH_shmat(seg2, 0, 0);
		pid			= information.pid;
		vt_page		= information.virtual_page;
		//achar um frame livre pra esse par processo-pagina virtual
	}
	else{
		
	}
	print "SIGNAL: %d\n", signal;
}

bool trans(pid_t pid, u_short i, u_short offset, char rw){
	//abrir mem. compartilhada do processo
	//percorrer table de memoria compartilhada checando se possui mapeamento
	int 	segmento = get_segmento(pid);
	u_int	entry = look_table(segmento, i, _left_);
	long	sleeper = (long)(pid & 0x00000FFF);


	if(entry == _max_){
		//se nao, avisa o GM que houve pagefault
		//salva o numero do processo requerinte e pagina virtual nao mapeada em uma outra memoria compartilhada(precisa ser criada pelo processo pai)
		if(isLocked_info()){
			sleep_nano(sleeper);
		}
		unlock_info(pid, i);
		kill(ppid(), SIGUSR1);
		raise(SIGSTOP);
		return false;
	}
	else {
		//se sim, imprime:
		printf("%d, %04x, %04x, %c\n", pid, (u_short)entry, offset, rw);
		/**
		 * CHECAR SE OS PARAMETROS DO PRINTF ESTAO CERTOS.
		 * 
		 * 
		 * 
		 * 
		 * 
		 */
		return true;
	}
}

u_int look_table(int segmento, u_short number, int side){
	int i;
	u_int 	entry;
	u_int*	table = (u_int*)EH_shmat(segmento, 0, 0);
	for(i = 0; i < _max_pages_; i++ ) {
		entry = table[i] & side; // entry = 16 bits do $side de table[i]
		if (to_side(entry, side) == number){ // se achar o numero, retorna o valor do outro lado do vetor de bits.
			return to_side(entry, ~side);
		}
	}
	return _max_;		
}

u_int get_segmento(pid_t pid){
	int i;
	for (i = 0 ; i < _n_of_process_ ; i++){
		if(pair_pid[i][PID_] = pid){
			return pair_pid[i][SEG_];
		}
	}

	print "Segmento invalido: [%d]\n", 
}

u_short to_side(u_int valor, int side){
	u_int a = (valor & side);
	if(side == _left_){
		return (u_short)( (valor & _left_ ) >> 16);
	}
	else if(side == _right_){
		return (u_short) (valor & _right_ );
	}
	print "Valor invalido para o lado.\n";
	exit(1);
}

void sleep_nano(long nanoseconds){
	struct timespec t;
	t->tv_sec = 0;
	t->tv_nsec = nanoseconds;
	nanosleep(t,NULL);
}

void sleep_ms(long ms){
	sleep_nano(1000000L * ms);
}

void lock_info(pid_t p, u_short vt_page){
	shd_info->pid = p;
	shd_info->virtual_page = vt_page;
}

void unlock_info(){
	shd_info->pid = 0;
}

bool isLocked_info(){
	if(shd_info->pid == 0){
		return false;
	}
	return true;
}