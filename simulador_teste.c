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


#define CPU_TIME 300.0

#define INV_INDEX -1

#define _n_of_process_	0x4	//4
#define _max_pages_	0x100	//256

#define _left_		0xffff0000
#define _right_		0x0000ffff
#define _max_		0xffffffff

#define _flags_ O_CREAT | S_IRUSR | S_IWUSR //O_CREAT | S_IRUSR | S_IWUSR

#define SLEEP_TIMER	(double)CPU_TIME

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


int			fault_key = 7100, pair_key = 7200;
int 		process_key;
int			process_shm[_n_of_process_] = { 8000 , 10000 , 12000 , 14000 };
Fault_Info*	shd_info;
u_int*		*tables__;
Access_Info*	accessed_info;
Fault_Info** vPhysicalMemory;
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
	pair_pid = create_shared_matrix( _n_of_process_ , 2 );
	vPhysicalMemory = (Fault_Info**)malloc(_max_pages_ * sizeof(Fault_Info*));

	EH_signal( SIGUSR1, sig_handler);


	segment  = 	EH_shmget(IPC_PRIVATE, sizeof(Fault_Info), _flags_);
	shd_info = 	EH_shmat(segment, NULL, 0);

	segment2 = 		EH_shmget(IPC_PRIVATE, sizeof(Access_Info), _flags_);
	accessed_info = EH_shmat(segment2, NULL, 0);
	accessed_info->pid = 0;
	shd_info->pid = 0;

	print "Iniciando\n");
	for( i = 0 ; i < _n_of_process_; i++ ){
		//print "Criando process [ %d / %d ]\n", i+1, _n_of_process_);
		pid = EH_fork();
		if( pid == 0 ) {
			pair_pid[i][PID_] = getpid();
			pair_pid[i][TABLE_] = i;
			create_process(process_names[i], i+1);
		}
	}

	start = now();
	elapsed = start;
	pid_timer = EH_fork();
	if(pid_timer > 0){
		while(true){
			print "Dormindo por [ %3.2lf ]ms\n", SLEEP_TIMER);
			sleep_ms(SLEEP_TIMER * 10);
			sleep(1);
			print "Liberando Cache\n");
			//clear_cache();
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

	file 		= EH_fopen(arquivo,"r");
	pid 		= getpid();
	sleep_timer = ((double)sleeper)/100;
	EH_signal( SIGUSR2, sig_handler_child);

	sleep(1);
	printf("Criando processo [%d]\n", getpid());
	while(fscanf(file,"%x %c ", &addr, &rw) != 0){
		//sleep_nano(sleeper); acho bom colocar para garntir que haja a alternancia entre os processos
		i = to_side(addr, _left_);
		o = to_side(addr, _right_);

		if(!trans(pid,i,o,rw)){
			while(shd_info->pid != 0){
				sleep_ms(sleep_timer);
				sleeper_extra(pid);
				if(blockcount > 1000){
					print "[Page_Fault] Preso num Deadlock - [ %d ]\n", shd_info->pid);
					sleep(1);
				}
				blockcount++;
				if(blockcount % 10 == 0){
					if(shd_info->pid == getpid()){
						break;
					}
				}
			}
			print "Liberando\n");
			lock_info(getpid(), i,rw);
			kill(getppid(), SIGUSR1);
			sleep(1);
		}
		sleep_ms(sleep_timer);
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

	if(!isLocked_info()){
		print "Recebido Sinal [%d] sinal enquanto processo bloqueado.\n\n\n\n");
		return;
	}
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
		if(frame == INV_INDEX){
			/*
				Só faz swap se nao tiver espaço na memória física.
			*/
			frame 		= get_least_frequency();
			if(vPhysicalMemory[frame] == NULL){
				print "Tentativa de acessar um frame nao inicializado.\n");
				unlock_info();
				return;
			}
			swap2_table	= get_table(vPhysicalMemory[frame]->pid);
			for(i = 0; i < _max_pages_ ; i++){
				if( getbytes(swap2_table[i],4) == getbytes(frame,4) )
				//if( (u_char) swap2_table[i] == frame) /*possivel erro*/
				{
					break;
				}
			}
			//wr = //( swap2_table[i] & 0x0000ff00 ) >> 2; //testar
			wr = (getbytes(swap2_table[i], 3)) >> 2;
			if(wr == 0x01){
				kill(vPhysicalMemory[frame]->pid,SIGUSR2);
				swap2_table[i] = 0xffffffff;
			}
			print "Iniciando processo de Swap.\n");
		}
		/*
			Salva na memória física.
		*/
		vPhysicalMemory[frame] = (Fault_Info*)malloc(sizeof(Fault_Info));
		vPhysicalMemory[frame]->pid = pid;
		vPhysicalMemory[frame]->virtual_page = vt_page;
		vPhysicalMemory[frame]->wr = shd_info->wr;

		for(i=0; i < _max_pages_ ; i++){
			if(vPhysicalMemory[i] != NULL){
				printf("[%d][%x][%c]\n", vPhysicalMemory[i]->pid,vPhysicalMemory[i]->virtual_page,vPhysicalMemory[i]->wr);
			}/*
			else{
				printf("=\t");
			}*/
		}
		printf("\n");
		sleep(1);

		/*
			Salva na tabela do processo.
		*/
		table[pos]= ( (u_int)vt_page << 16 ) & 0xffff0000;

		aux = (u_int)frame; // ??????????

		if(shd_info->wr == true){
			aux |= 0x00000100;
		}
		table[pos] = table[pos] & aux;
		access_addr( table[pos] & 0x000000ff);
		print "Liberando processo: %d\n", pid);
		kill(pid, SIGCONT);
		unlock_info();
	}
	else{
		print "Invalid Signal\n");
	}
}

void sig_handler_child(int signal){
	if(signal == SIGUSR2){
		print "Recebido [ SIGUSR2 ]\n");
		sleep(2);
	}
}

u_int** create_shared_matrix(u_int nTables, u_int tableSize){
	int 	i,j;
	u_int**	table__;
	int 	seg = EH_shmget(IPC_PRIVATE, nTables * sizeof(u_int), _flags_);
	table__ = EH_shmat( seg , 0 , 0 );
	for(i = 0; i < nTables; i++ ){
		seg = EH_shmget(IPC_PRIVATE, (1 + tableSize) * sizeof(u_int), _flags_);
		table__[i] = EH_shmat( seg , 0 , 0 );
		for(j=0; j < tableSize + 1; j++){
			table__[i][j] = 0;
		}
	}
	return table__;	
}

bool trans(pid_t pid, u_short i, u_short offset, char rw){
	int* 	table = get_table(pid);
	Fault_Info information;
	u_int	entry = look_table(table, i, _left_);
	long	sleeper = (long)(pid & 0x00000FFF);
	int 	lockcount = 0;

	int out,k;
	//printf("\n\nTabela(%d)\n",pid);
	for(out = 0; out < _max_pages_; out++){
		table[out]++;
		//printf("%d ", table[out]);
	}
	//printf("\n");
	//printf("Antes: %x\n", i);
	//printf("Depois: %04x\n\n", entry);
	if(entry == _max_){
		print "Endereço virtual nao encontrado na tabela. Lancando Page-Fault.\n");
		return false;
	}
	else {
		//se sim, imprime:
		print "%d, %04x, %04x, %c\n", pid, entry, offset, rw);
		//tenho q colocar pra se o acesso for de escrita mudar os bytes marcados com A 0x0000AA00 A para 01 escrita 
		//inicialmente se for aberto somente pra leitura ele vai estar 00 entao precisa mudar por causa do swap2
		if(accessed_info->pid != 0){
			while(accessed_info->pid != 0){
				lockcount++;
				sleep_ms(1);
				if(lockcount > 1000){
					print "[Access] Preso em um Deadlock. - [ %d ]\n", accessed_info->pid);
				}
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
		entry = table[i] & side; // entry = 16 bits do $side de table[i]
		if (to_side(entry, side) == number){ // se achar o numero, retorna o valor do outro lado do vetor de bits.
			return to_side(entry, ~side);
		}
	}
	return _max_;		
}

u_int* get_table(pid_t pid){
	int i = INV_INDEX;
	int index = INV_INDEX;
	for (i = 0 ; i < _n_of_process_ ; i++){
		if(pair_pid[i][PID_] = pid){
			return tables__[pair_pid[i][TABLE_]];
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
		if(table[i] == 0){
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