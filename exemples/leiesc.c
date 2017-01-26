#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <semaforo.h>

#define COM_SEMAFOROS 1
#if COM_SEMAFOROS
	#define SEM__CREATE(x) sem_create(x)
	#define SEM__DESTROY(x) sem_destroy(x)
	#define SEM__P(x) sem_p(x)
	#define SEM__V(x) sem_v(x)
#else
	#define SEM__CREATE(x)
	#define SEM__DESTROY(x)
	#define SEM__P(x)
	#define SEM__V(x)
#endif

int N_ESC = 2;
int N_LEI = 10;
int ITER = 5;
#define DEBUG(x) 

int s;
int sem_escrita;
int sem_leitura;
int sem_mutex;

/* Variaveis Globais */
#define ARQUIVO_GLOBAIS "leiesc.globais"
#define TAM_FILA 1000
struct 
{
	char fila[TAM_FILA];
	int inic_fila;
	int fim_fila;
	int leitores_ativos;
	int escritor_ativo;
	int escritores_esperando;
	int leitores_esperando;
} globais;

void le_globais(void)
{
	int fd;
	int i;
	fd = open(ARQUIVO_GLOBAIS, O_RDONLY);
	read(fd, &globais, sizeof(globais));
	close(fd);
	/**/
	
	
	printf("\nLeitoresAtivos=%d, EscritorAtivo=%d, LeitoresEsperando=%d, EscritoresEsperando=%d\n", globais.leitores_ativos, globais.escritor_ativo, globais.leitores_esperando, globais.escritores_esperando);
	
	if	(((globais.leitores_ativos > 0) && (globais.escritor_ativo > 0)) ||
		(globais.escritor_ativo > 1) ||
		(globais.escritor_ativo < 0) ||
		(globais.leitores_ativos < 0))
	{
		printf("ERRO!!! ASSERTIVAS\n");
		SEM__P(sem_mutex);
	}

	
}

void esc_globais(void)
{
	int fd;	
	if	(((globais.leitores_ativos > 0) && (globais.escritor_ativo > 0)) ||
		(globais.escritor_ativo > 1) ||
		(globais.escritor_ativo < 0) ||
		(globais.leitores_ativos < 0))
	{
		printf("ERRO!!! ASSERTIVAS (gravacao)\n");
		SEM__P(sem_mutex);
	}
	fd = open(ARQUIVO_GLOBAIS, O_CREAT | O_WRONLY);
	write(fd, &globais, sizeof(globais));
	close(fd);
}
/* fim Variaveis Globais */



void pre_ler(void) 
{ 
	SEM__P(sem_mutex);
	le_globais();
	
	if (globais.escritor_ativo + globais.escritores_esperando == 0)
	{
		globais.leitores_ativos++;
		SEM__V(sem_leitura);
	} else {
		globais.leitores_esperando++;
	}
	esc_globais();
	SEM__V(sem_mutex);
	SEM__P(sem_leitura);
}
void pos_ler(void) 
{ 
	SEM__P(sem_mutex);
	le_globais();
	globais.leitores_ativos--;
	if ((globais.leitores_ativos == 0) && (globais.escritores_esperando > 0))
	{
		SEM__V(sem_escrita);
		globais.escritor_ativo++;
		globais.escritores_esperando--;
	}

	esc_globais();
	SEM__V(sem_mutex);
}
void pre_escrever(void) 
{ 
	SEM__P(sem_mutex);
	le_globais();
	if (globais.escritor_ativo + globais.leitores_ativos + globais.leitores_esperando == 0)
	{
		globais.escritor_ativo++;
		SEM__V(sem_escrita);
	} else {
		globais.escritores_esperando++;
	}
	esc_globais();
	SEM__V(sem_mutex);
	SEM__P(sem_escrita);
}
void pos_escrever(void) 
{ 
	SEM__P(sem_mutex);
	le_globais();
	globais.escritor_ativo--;
	if ((globais.escritor_ativo == 0) && (globais.leitores_esperando > 0))
	{
		while(globais.leitores_esperando > 0)
		{
			SEM__V(sem_leitura);
			globais.leitores_ativos++;
			globais.leitores_esperando--;
		}
	} else if ((globais.escritor_ativo == 0) && (globais.escritores_esperando > 0)){
		SEM__V(sem_escrita);
		globais.escritor_ativo++;
		globais.escritores_esperando--;
	}

	esc_globais();
	SEM__V(sem_mutex);
}

void escritor(int id)
{
	int i;
	char msg1[50];
	char msg2[50];
	
	sprintf(msg1, "%2d Comecou a escrever\n", id);
	sprintf(msg2, "%2d Terminou de escrever\n", id);
	
	for (i = 0; i < ITER; i++)
	{
		pre_escrever();
		write(s, msg1, 22);
		printf("%2d Escrevendo\n", id);
		
		/* demora escrevendo para ser preemptado */
		sleep(1);
		printf("%2d Terminou de escrever\n", id);
		write(s, msg2, 24);
		pos_escrever();
		
		/*Pensa*/
		sleep(1);
	}
}

void leitor(int id)
{
	int i;
	char msg1[50];
	char msg2[50];
	
	sprintf(msg1, "%2d Comecou a ler\n", id);
	sprintf(msg2, "%2d Terminou de ler\n", id);
	for (i = 0; i < ITER; i++)
	{
		pre_ler();
		write(s, msg1, 17);
		printf("%2d Lendo\n", id);
		
		/* demora lendo para ser preemptado */
		sleep(1);
		
		write(s, msg2, 19);
		pos_ler();
		
		/*Pensa*/
		sleep(1);
	}
}

int main(int argc, char * argv[])
{
	int i;
	int pid;
	int fd;


	if ((argc != 4) && (argc != 1))
	{
		printf("Uso do programa: leiesc num_leitores num_escritores num_iteracoes\n");
		_exit(1);
	}
	if (argc == 4)
	{
		if (sscanf(argv[1], "%d", &N_LEI) != 1) _exit(1);
		if (sscanf(argv[2], "%d", &N_ESC) != 1) _exit(1);
		if (sscanf(argv[3], "%d", &ITER) != 1) _exit(1);
	}	
	
	
	memset(&globais, 0, sizeof(globais));
	esc_globais();
	#if COM_SEMAFOROS
	sem_leitura = SEM__CREATE(0);
	sem_escrita = SEM__CREATE(0);
	sem_mutex = SEM__CREATE(1);
	#endif
	s = open("leiesc.saida",  O_CREAT | O_WRONLY);
	for (i = 1; i <= N_ESC; i++)
	{
		pid = fork();
		if (pid < 0) return 1;
		if (pid == 0)
		{
			printf("Criando escritor %d\n", i);
			escritor(i);
			return 0;
		}
	}
	
	for (i = 1; i <= N_LEI; i++)
	{
		pid = fork();
		if (pid < 0) return 1;
		if (pid == 0)
		{
			printf("Criando leitor %d\n", i);
			leitor(i);
			return 0;
		}
	}
	
	for (i = 0; i < N_LEI+N_ESC; i++)
		wait(NULL);
	close(s);
	
	SEM__DESTROY(sem_leitura);
	SEM__DESTROY(sem_escrita);
	SEM__DESTROY(sem_mutex);
	return 0;
}

