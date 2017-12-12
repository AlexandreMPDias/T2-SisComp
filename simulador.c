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
#include <sys/sem.h>
 #include <sys/wait.h>
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


#define CPU_TIME 300.0

#define INV_INDEX -1

#define _n_of_process_	0x4	//4
#define _max_pages_	0x100	//256

#define _left_		0xffff0000
#define _right_		0x0000ffff
#define _max_		0xffffffff
#define SEM 7300
#define _flags_ O_CREAT | S_IRUSR | S_IWUSR //O_CREAT | S_IRUSR | S_IWUSR

#define SLEEP_TIMER	(double)CPU_TIME
 union semun
{
	int              val;
	unsigned short  *array;
	struct semid_ds *buf;
};
int setSemValue( int semId );

void delSemValue( int semId );

int semaforoP( int semId );

int semaforoV( int semId );
//----->> estrutura com informaçoes sobre o page-fault
typedef struct Fault_Info__{
	pid_t pid;
	u_short virtual_page;
	bool wr;
}Fault_Info;

typedef struct Access_Info__{
	pid_t pid;
	u_int addr;
}Access_Info;
//---->> Variaveis Globais

/*
 * Vetor alocado na memória compartilhada para realizar o mapeamento entre o process_id e o segmento em que a page_table desse processo está alocada.
 */
u_int** pair_pid;
int semId;

int			fault_key = 7100, pair_key = 7200;
int 		process_key;
int			process_shm[_n_of_process_] = { 8000 , 10000 , 12000 , 14000 };
Fault_Info*	shd_info;
u_int*		*tables__;
Access_Info*	accessed_info;
Fault_Info** vPhysicalMemory;
int 		my_table;
int numero_do_processo;
int pid_processos[4];
int numero_de_pagefaults;
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
 * @return em caso da tabela não ter nenhum endereço vazio, a função retornará [ INV_INDEX ]
 */
int find_empty_spot(u_int* table);
/**
 * find_empty_spot
 * Percorre o vetor de frames da memoria fisica, e retorna o primeiro frame nao alocado.
 * @param table que deverá ser percorrida.
 * 
 * @return em caso da tabela não ter nenhum endereço vazio, a função retornará [ INV_INDEX ]
 */
int find_empty_spot(u_int* table);
int find_empty_spot_physical();

/**
 * Funções criadas para simular um semaforo.
 */
void lock_info(pid_t,u_short,bool wr);
void unlock_info();
bool isLocked_info();

/**
 * Funções para fazer o programa dormir milisegundos.
 */
void sleep_ms(double ms);
void sleeper_extra(int val);
double now();

u_int getbytes(u_int val, int byte);

void sig_handler(int signal);

void sig_handler_child(int signal);

int main(void){
	int 		i, segment, segment2;
	pid_t		pid, pid_timer;
	char 		*process_names[100] = { "compilador.log" , "compressor.log" , "matriz.log" , "simulador.log" };
	long		start, elapsed;
	tables__ = create_shared_matrix( _n_of_process_ , _max_pages_ );
	numero_de_pagefaults=0;
	vPhysicalMemory = (Fault_Info**)malloc(_max_pages_ * sizeof(Fault_Info*));

	EH_signal(SIGUSR1, sig_handler);


	segment  = 	EH_shmget(IPC_PRIVATE, sizeof(Fault_Info), _flags_);
	shd_info = 	EH_shmat(segment, NULL, 0);

	segment2 = 		EH_shmget(IPC_PRIVATE, sizeof(Access_Info), _flags_);
	accessed_info = EH_shmat(segment2, NULL, 0);
	accessed_info->pid = 0;
	shd_info->pid = 0;

	semId = semget( SEM, 1, 0666 | IPC_CREAT );
	if( semId == -1 )
	{
		printf("interpretador.c: Nao foi possivel criar o semaforo.\n");
		exit( 1 );
	}

	if( setSemValue( semId ) == -1 )
	{
		printf("interpretador.c: Nao foi possivel operar o semaforo.\n");
		exit( 1 );
	}
	print "Iniciando\n");
	create_cache();
	for( i = 0 ; i < _n_of_process_; i++ ){

		numero_do_processo=i;
		pid = EH_fork();
		if( pid == 0 ) {

			my_table = i;
			create_process(process_names[i], i+1);
			print "Encerrando leitura");
		}
		else{
			pid_processos[i]=pid;
		}
	}

	start = now();
	elapsed = start;
	pid_timer = EH_fork();
	if(pid_timer > 0){
		while(true){
			print "Dormindo por [ %3.2lf ]ms\n", SLEEP_TIMER);
			//sleep_ms(SLEEP_TIMER * 10);
			usleep(600);
			print "Liberando Cache\n");
			clear_cache();
		}
	}
	else{
		while(true){
			if(accessed_info->pid != 0){
				access_addr(accessed_info->addr);
				accessed_info->pid = 0;
			}
		}
	}
}

void create_process(char* arquivo, u_int sleeper){
	u_int		addr;
	u_short		i , o;
	pid_t		pid;
	char		rw;
	FILE*		file;
	int 		blockcount = 0;
	double 		sleep_timer;
	bool		page_faulted;

	file 		= EH_fopen(arquivo,"r");
	pid 		= getpid();
	sleep_timer = ((double)sleeper)/100;

	sleep(1);
	EH_signal( SIGUSR2, sig_handler_child);
	EH_signal( SIGUSR1, sig_handler_child);
	page_faulted = false;

	printf("Criando processo [%d]\n", getpid());
	while(fscanf(file,"%x %c ", &addr, &rw) != 0){
		usleep(10);
		i = to_side(addr, _left_);
		o = to_side(addr, _right_);
		page_faulted = !trans(pid,i,o,rw);
		if(page_faulted){
			semaforoP( semId );
			print "Liberando\n");
			lock_info(getpid(), i,rw);
			kill(getppid(), SIGUSR1);
			usleep(10);
		}
		//sleeper_extra(sleep_timer);

		if(page_faulted){
			//sleep(1);
			usleep(300000);/* so pra ir mais rapido*/
			page_faulted = !page_faulted;
		}
	}
	fclose(file);
}

void sig_handler(int signal){
	int			seg1, seg2;
	int			pos, frame, i;
	pid_t		pid;

	u_short		vt_page;
	u_int*		table;
	u_int*		swap2_table;
	u_int		aux;
	u_char		wr;

	pid		= shd_info->pid;
	vt_page	= shd_info->virtual_page;

	if(signal == SIGUSR1){
		print "Recebido [ SIGUSR1 ] pelo processo [ %d ]\n", pid);
		kill(pid, SIGSTOP);
		table	= get_table(pid);
		frame	= find_empty_spot_physical();
		print "Frame livre [ %d ]\n", frame);
		//find_empty_spot retorna um index na tabela. Frame = o index?

		pos		= find_empty_spot(table);
		numero_de_pagefaults++;
		printf("numero de pagefaults: %d\n",numero_de_pagefaults);
		if(frame == INV_INDEX){
			/*
				Só faz swap se nao tiver espaço na memória física.
			*/
			frame 		= get_least_frequency();
			printf( "Frame com menor frequencia[%d].\n",frame);
			if(vPhysicalMemory[frame] == NULL){
				print "Tentativa de acessar um frame nao inicializado.\n");
				unlock_info();
				return;
			}
			swap2_table	= get_table(vPhysicalMemory[frame]->pid);
			for(i = 0; i < _max_pages_ ; i++){
				if( getbytes(swap2_table[i],4) == getbytes(frame,4) )
				{
					break;
				}
			}
			wr = (getbytes(swap2_table[i], 3)) >> 2;
			//printf("addr: %x wr: %x\n",swap2_table[i],wr);
			if(wr != 0){
				kill(vPhysicalMemory[frame]->pid,SIGUSR2);
				//kill(pid,SIGUSR1);
			}
			swap2_table[i] = 0xffffffff;
			print "Iniciando processo de Swap.\n");
			free(vPhysicalMemory[frame]);
		}
		/*
			Salva na memória física.
		*/
		vPhysicalMemory[frame] = (Fault_Info*)malloc(sizeof(Fault_Info));
		vPhysicalMemory[frame]->pid = pid;
		vPhysicalMemory[frame]->virtual_page = vt_page;
		vPhysicalMemory[frame]->wr = shd_info->wr;

		table[pos]= ((u_int)vt_page) << 16;

		aux = (u_int)frame; 

		if(shd_info->wr == true){
			aux |= 0x00000100;
		}
		table[pos] = table[pos] + aux;
		access_addr( table[pos] & 0x000000ff);
		print "Liberando processo: %d\n", pid);
		kill(pid, SIGCONT);
		semaforoV( semId );
	}
	else{
		print "Invalid Signal\n");
	}
}

void sig_handler_child(int signal){
	if(signal == SIGUSR2){
		print "Recebido [ SIGUSR2 ]\n");
	}
	else{
		printf("vai ter q salvar primeiro na memoria fisica, espere!!\n----------\n---------\n----\n");
		sleep(1);
	}
}

u_int** create_shared_matrix(u_int nTables, u_int tableSize){
	int 	i,j;
	u_int**	table__;
	int 	seg;// = EH_shmget(IPC_PRIVATE, nTables * sizeof(u_int*), _flags_);
	//table__ = EH_shmat( seg , 0 , 0 );
	table__ = (u_int**)malloc(sizeof(u_int*) * nTables);
	for(i = 0; i < nTables; i++ ){
		seg = EH_shmget(IPC_PRIVATE, (tableSize) * sizeof(u_int), _flags_);
		table__[i] = EH_shmat( seg , 0 , 0 );
		for(j = 0; j < tableSize; j++){
			table__[i][j] = 0xFFFFFFFF;
		}
	}
	return table__;	
}

bool trans(pid_t pid, u_short i, u_short offset, char rw){
	u_int* 	table = tables__[numero_do_processo];
	Fault_Info information;
	int	entry = look_table(table, i, _left_);
	long	sleeper = (long)(pid & 0x00000FFF);
	int 	lockcount = 0;

	//printf("Numero do Processo: [%d]\n",numero_do_processo);
	if(entry ==-1){
		print "(%d) Endereço virtual nao encontrado na tabela. Lancando Page-Fault.\n", pid);
		printf("(%d) Endereço virtual nao encontrado na tabela. Lancando Page-Fault\n", pid);
		return false;
	}
	else {
		if(rw=='W'){
			table[entry]=table[entry]|0x00000100;
		}
		access_addr( table[entry] & 0x000000ff);
		//se sim, imprime:
		printf("(%d):\t%-02d\t%04x\t%c\n", pid, table[entry] & 0x000000ff, offset, rw);
		//tenho q colocar pra se o acesso for de escrita mudar os bytes marcados com A 0x0000AA00 A para 01 escrita 
		//inicialmente se for aberto somente pra leitura ele vai estar 00 entao precisa mudar por causa do swap2
		while(accessed_info->pid != 0){
			lockcount++;
			//sleeper_extra(pid/2);
			if(lockcount > 1000){
				print "[Access] Preso em um Deadlock. - [ %d ]\n", accessed_info->pid);
			}
		}
		accessed_info->pid = pid;
		accessed_info->addr = entry;
	}
    return true;
}

u_int look_table(u_int* table, u_short number, int side){
	int 	i;
	u_int 	entry;
	for(i = 0; i < _max_pages_; i++ ) {
		entry = to_side(table[i],side);
		if ((u_short)entry == number){ //to_side(entry, side) se achar o numero, retorna o valor do outro lado do vetor de bits.
			return i;
		}
	}
	return -1;		
}

u_int* get_table(pid_t pid){
	int i = INV_INDEX;
	int index = INV_INDEX;
	for (i = 0 ; i < _n_of_process_ ; i++){
		if(pid == pid_processos[i]){
			return tables__[i];
		}
	}
	print "Tabela não encontrada.\n");
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
		if(table[i] == _max_){
			return i;
		}
	}
	print "Nenhum valor vazio na tabela foi encontrado.\n");
	return INV_INDEX;
}

int find_empty_spot_physical(){
	int i;
	for(i = 0; i < _max_pages_ ; i++ ){
		if(vPhysicalMemory[i] == NULL){
			return i;
		}
	}
	print "Nenhum valor vazio na memoria fisica foi encontrado.\n");
	return INV_INDEX;
}

void sleep_ms(double ms){
	long s = (long)(ms * 1000.0);
	usleep(s);
}

void lock_info(pid_t p, u_short vt_page,bool wr){
	if(p == 0){
		print "Recebendo Pid = 0\n");
	}
	shd_info->pid = p;
	shd_info->virtual_page = vt_page;
	shd_info->wr=wr;
	print "Bloqueando acesso ao Page_Fault_Handler\n");
}

void unlock_info(){
	shd_info->pid = 0;
	print "Liberando acesso ao Page_Fault_Handler\n");
}

bool isLocked_info(){
	if(shd_info->pid == 0){
		return false;
	}
	return true;
}

u_int getbytes(u_int val, int byte){
	if(byte == 1){
		return val & 0xff000000;
	}
	else if(byte == 2){
		return val & 0x00ff0000;
	}
	else if(byte == 3){
		return val & 0x0000ff00;
	}
	else if(byte == 4){
		return val & 0x000000ff;
	}
	else{
		print "Byte = [%d], Valid values range from 1 to 4\n");
		return 0;
	}
}

double now(){
	struct timeval t1;
	double elapsedTime;
    gettimeofday(&t1, NULL);
    elapsedTime = (t1.tv_sec) * 1000.0;
    elapsedTime += (t1.tv_usec) / 1000.0; 
	return elapsedTime;
}

void sleeper_extra(int val){
	int timer = ((11*(val * val) + val)) & 0x0000FFFF;
	int i;
	for(i=0; i < timer; i++);
}
int setSemValue( int semId )
{
	union semun semUnion;

	semUnion.val = 1;

	return semctl( semId, 0, SETVAL, semUnion );
}



void delSemValue( int semId )
{
	union semun semUnion;

	semctl( semId, 0, IPC_RMID, semUnion );
}



int semaforoP( int semId )
{
	struct sembuf semB;

	semB.sem_num =  0;
	semB.sem_op  = -1;
	semB.sem_flg = SEM_UNDO;

	semop( semId, &semB, 1 );

	return 0;
}



int semaforoV(int semId)
{
	struct sembuf semB;

	semB.sem_num = 0;
	semB.sem_op  = 1;
	semB.sem_flg = SEM_UNDO;

	semop( semId, &semB, 1 );

	return 0;
}