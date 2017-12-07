/*
 * Sistemas de Computação - INF1019 - 2017.2
 * Prof Markus Endler
 * Trabalho 2 - Simulando Memória Virtual e Substituição de Páginas LFU
 * 
 * Aluno1: Alexandre de Mello. P Dias - 1413183
 * Aluno2: Caio Freiertag - xxxxxxxxxxx
 * 
 */

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/shm.h>

#include "error_handler.h"
#include "auxiliar.h"
#include "page_structure.h"


#define print debug(PRE, 0,

#define PID_ 0
#define TABLE_ 1
#define CPU_TIME 30

#define _n_of_process_	0x4	//4
#define _max_pages_	0x100	//256

#define _left_		0xffff0000
#define _right_		0x0000ffff
#define _max_		0xffffffff

#define SLEEP_TIMER	30

//----->> estrutura com informaçoes sobre o page-fault
typedef struct Fault_Info__{
	int pid;
	u_short virtual_page;
}Fault_Info;

//---->> Variaveis Globais

/*
 * Vetor alocado na memória compartilhada para realizar o mapeamento entre o process_id e o segmento em que a page_table desse processo está alocada.
 */
u_int** pair_pid;


int		fault_key = 7100, pair_key = 7200;
int 		process_key;
int		process_shm[_n_of_process_] = { 8000 , 10000 , 12000 , 14000 };
Fault_Info*	shd_info;
u_int*		*tables__;

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
 * get_table
 * Percorre a lista de tabelas para cada processo e retorna a table desse processo.
 */
u_int* get_table(pid_t pid);

/**
 * look_table
 * Percorre a page_table to processo e procurando por um determinado valor de 16 bits dentro do endereco na tabela.
 * @param table: a propria table
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
u_int look_table(u_int* table, u_short number, int side);

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

/**
 * create_shared_matrix
 * Cria uma matrix de ( nTables linhas e tableSize colunas ) na memória compartilhada.
 * Inicializa todos os valores com 0.
 * 
 */
u_int** create_shared_matrix(u_int nTables, u_int tableSize);

/**
 * find_empty_spot
 * Percorre uma tabela, e retorna o primeiro endereço vazio.
 * @param table que deverá ser percorrida.
 * 
 * @return em caso da tabela não ter nenhum endereço vazio, a função retornará [ -1 ]
 */
int find_empty_spot(u_int* table);

/**
 * Funções criadas para simular um semaforo.
 */
void lock_info(pid_t,u_short);
void unlock_info();
bool isLocked_info();

/**
 * Funções para fazer o programa dormir nanosegundos ou milisegundos.
 */
void sleep_nano(long nanoseconds);
void sleep_ms(long ms);

void sig_handler(int signal);

int main(void){
	int 		i, segment, s;
	pid_t		pid;
	Fault_Info	information;
	char 		*process_names[100]={ "compilador.log" , "compressor.log" , "matriz.log" , "simulador.log" };
	struct timeval	start_tv,corr_tv;
	
	tables__ = create_shared_matrix( _n_of_process_ , _max_pages_ );
	pair_pid = create_shared_matrix( _n_of_process_ , 2 );

	EH_signal( SIGUSR2, sig_handler);
	EH_signal( SIGUSR1, sig_handler);


	segment = EH_shmget(IPC_PRIVATE, sizeof(Fault_Info), O_CREAT | S_IRUSR | S_IWUSR);
	shd_info = EH_shmat(segment,NULL,0);

	
	for( i = 0 ; i < _n_of_process_; i++ ){
		print "Criando process [ %d / %d ]\n", i+1, _n_of_process_);
		pid = EH_fork();
		if( pid == 0 ){
			//process_key = process_shm[i];
			//segment = EH_shmget(process_shm[i], _max_pages_ * sizeof(u_int), IPC_CREAT | S_IRUSR | S_IWUSR);
			pair_pid[i][PID_] = getpid();
			pair_pid[i][TABLE_] = i;
			create_process(process_names[i], i+1);
		}
	}
	gettimeofday (&start_tv, NULL);
	while(true){
		gettimeofday (&corr_tv, NULL);
		if(((corr_tv.tv_usec-start_tv.tv_usec) > CPU_TIME)){
			start_tv = corr_tv;
			clear_cache();
		}
		else{
			print "Sleeping for %d miliseconds.", SLEEP_TIMER);
			sleep_ms(SLEEP_TIMER);
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

		if(!trans(pid,i,o,rw)){
			while(isLocked_info()){
				sleep_nano(sleeper);
			}
			print "Desprendendo Page_Fault Handler.\n");
			lock_info(pid, i);
			kill(getppid(), SIGUSR1);
			raise(SIGSTOP);
		}
	}
	fclose(file);
}

void sig_handler(int signal){
	int		seg1, seg2;
	pid_t		pid;
	u_short		vt_page;
	int		frame;

	u_int*		table;

	Fault_Info information = *shd_info;
	
	print "Sinal recebido.");
	if(signal == SIGUSR1){
		// seg1		= EH_shmget(fault_key, sizeof(Fault_Info),  S_IRUSR | S_IWUSR);
		// information	= (Fault_Info)EH_shmat(seg1, 0, 0);
		//seg2		= EH_shmget(FP, _max_pages_ * sizeof(u_int),  S_IRUSR | S_IWUSR);
		//table 		= (u_int *)EH_shmat(seg2, 0, 0);
		pid		= shd_info->pid;
		vt_page		= shd_info->virtual_page;
		print "[ SIGUSR1 ] pelo processo [ %d ]\n", pid);

		table = get_table(pid);
		frame = get_table_spot(table);

		if(frame < 0){
			print "Iniciando processo de Swap.\n");
		}
		
		//achar um frame livre pra esse par processo-pagina virtual
		/**
		 * 
		 * 
		 * NAO ESTA FAZENDO NADA
		 * 
		 * 
		 */

		print "Desprendendo Page_Fault Handler.\n");
		unlock_info();
	}
	else{
		print "[ SIGUSR1 ] pelo processo [ %d ]\n", pid);
		/**
		 * 
		 * 
		 * FALTA A IMPLEMENTACAO DO SIGUSR2
		 * 
		 * 
		 */
		
	}
	print "SIGNAL: %d\n", signal);
}

u_int** create_shared_matrix(u_int nTables, u_int tableSize){
	int 	i,j;
	u_int**	table__;
	int 	seg = EH_shmget(IPC_PRIVATE, nTables * sizeof(u_int), O_CREAT | S_IRUSR | S_IWUSR);
	table__ = EH_shmat( seg , 0 , 0 );
	for(i = 0; i < nTables; i++ ){
		seg = EH_shmget(IPC_PRIVATE, (1 + tableSize) * sizeof(u_int), O_CREAT | S_IRUSR | S_IWUSR);
		table__[i] = EH_shmat( seg , 0 , 0 );
		for(j=0; j < tableSize + 1; j++){
			table_[i][j] = 0;
		}
	}
	return table__;	
}

bool trans(pid_t pid, u_short i, u_short offset, char rw){
	int* 	table = get_table(pid);
	Fault_Info information;

	u_int	entry = look_table(table, i, _left_);
	long	sleeper = (long)(pid & 0x00000FFF);


	if(entry == _max_){
		print "Endereço virtual nao encontrado na tabela.\nLancando Page-Fault.\n");
		return false;
	}
	else {
		//se sim, imprime:
		printf("%d, %04x, %04x, %c\n", pid, (u_short)entry, offset, rw);
		return true;
	}
}

u_int look_table(u_int* table, u_short number, int side){
	int i;
	u_int 	entry;
	for(i = 0; i < _max_pages_; i++ ) {
		entry = table[i] & side; // entry = 16 bits do $side de table[i]
		if (to_side(entry, side) == number){ // se achar o numero, retorna o valor do outro lado do vetor de bits.
			return to_side(entry, ~side);
		}
	}
	return _max_;		
}

u_int* get_table(pid_t pid){
	int i = -1;
	int index = -1;
	for (i = 0 ; i < _n_of_process_ ; i++){
		if(pair_pid[i][PID_] = pid){
			return tables__[pair_pid[i][TABLE_]];
			break;
		}
	}
	print "Tabela não encontrada\n");
	exit(1);
}

u_short to_side(u_int valor, int side){
	u_int a = (valor & side);
	if(side == _left_){
		return (u_short)( (valor & _left_ ) >> 16);
	}
	else if(side == _right_){
		return (u_short) (valor & _right_ );
	}
	print "Valor invalido para: [ lado ].\n");
	exit(1);
}

int find_empty_spot(u_int* table){
	int i;
	for(i = 0; i < _max_pages_ ; i++ ){
		if(table[i] == 0){
			return i;
		}
	}
	print "Nenhum valor vazio na tabela foi encontrado.\n");
	return -1;
}

void sleep_nano(long nanoseconds){
	struct timespec t;
	t.tv_sec = 0;
	t.tv_nsec = nanoseconds;
	nanosleep(&t,NULL);
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
