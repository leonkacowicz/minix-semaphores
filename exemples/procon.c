#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <semaforo.h>

#define DEBUG(x) 

int N_PROD = 15;
int N_CONS = 15;
int ELEM = 30;
int N_BUFFER = 50;


int s;
int sem1;
int sem2;
int mutex;

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

/* Variaveis Globais */
#define ARQUIVO_GLOBAIS "procon.globais"

struct
{
	int elem_produzidos;
	int elem_consumidos;
} globais;

void le_globais(void)
{
	int fd;
	fd = open(ARQUIVO_GLOBAIS, O_RDONLY);
	read(fd, &globais, sizeof(globais));
	close(fd);
}

void esc_globais(void)
{
	int fd;
	fd = open(ARQUIVO_GLOBAIS, O_CREAT | O_WRONLY);
	write(fd, &globais, sizeof(globais));
	close(fd);
}
/* fim Variaveis Globais */

void pre_ler(void) 
{ 
	SEM__P(sem1); 
	SEM__P(mutex); 
}
void pos_ler(void) 
{ 
	SEM__V(mutex);
	SEM__V(sem2); 
}
void pre_escrever(void) 
{ 
	SEM__P(sem2); 
	SEM__P(mutex);
}
void pos_escrever(void) 
{ 
	SEM__V(mutex);
	SEM__V(sem1); 
}

void produtor(int id)
{
	int eu_produzi = 0;
	char msg1[50];
	char msg2[50];
	
	sprintf(msg1, "%2d Comecou a produzir\n", id);
	sprintf(msg2, "%2d Terminou de produzir\n", id);
	
	while (1)
	{
		/* pensa */
		sleep(1);
		pre_escrever();
		le_globais();
		if (globais.elem_produzidos == ELEM) 
		{
			SEM__DESTROY(sem1);
			SEM__DESTROY(sem2);
			SEM__DESTROY(mutex);			
			break;
		}
		write(s, msg1, 22);
		/* Demora produzindo */
		sleep(1);
		globais.elem_produzidos++;
		eu_produzi++;
		printf("%d Produzindo\n", id);
		write(s, msg2, 24);
		esc_globais();
		pos_escrever();
		
	}
	printf("Produtor #%d produziu %d elementos\n", id, eu_produzi);
}

void consumidor(int id)
{
	int eu_consumi = 0;
	char msg1[50];
	char msg2[50];
	
	sprintf(msg1, "%2d Comecou a consumir\n", id);
	sprintf(msg2, "%2d Terminou de consumir\n", id);
	
	while (1)
	{
		pre_ler();
		le_globais();
		if (globais.elem_consumidos == ELEM) 
		{
			SEM__DESTROY(sem1);
			SEM__DESTROY(sem2);
			SEM__DESTROY(mutex);
			break;
		}
		write(s, msg1, 22); 
		printf("%d Consumindo\n", id);		
		globais.elem_consumidos++;
		
		eu_consumi++;
		write(s, msg2, 24);
		
		esc_globais();
		pos_ler();
		
		/* pensa */
		sleep(1);
		
	}
	printf("Consumidor #%d consumiu %d elementos\n", id, eu_consumi);
}

int main(int argc, char * argv[])
{
	int i;
	int pid;
	int fd;
	
	if ((argc != 5) && (argc != 1))
	{
		printf("Uso do programa: procon num_produtores num_consumidores num_elementos tam_buffer\n");
		_exit(1);
	}
	if (argc == 5)
	{
		if (sscanf(argv[1], "%d", &N_PROD) != 1) _exit(1);
		if (sscanf(argv[2], "%d", &N_CONS) != 1) _exit(1);
		if (sscanf(argv[3], "%d", &ELEM) != 1) _exit(1);
		if (sscanf(argv[4], "%d", &N_BUFFER) != 1) _exit(1);
	}
	
	memset(&globais, 0, sizeof(globais));
	esc_globais();
	#if COM_SEMAFOROS
	sem1 = SEM__CREATE(0);
	sem2 = SEM__CREATE(N_BUFFER);
	mutex = SEM__CREATE(1);
	#endif
	s = open("procon.saida",  O_CREAT | O_WRONLY);
	for (i = 1; i <= N_PROD; i++)
	{
		pid = fork();
		if (pid < 0) return 1;
		if (pid == 0)
		{
			printf("Criando produtor %d\n", i);
			produtor(i);
			return 0;
		}
	}
	
	for (i = 1; i <= N_CONS; i++)
	{
		pid = fork();
		if (pid < 0) return 1;
		if (pid == 0)
		{
			printf("Criando consumidor %d\n", i);
			consumidor(i);
			return 0;
		}
	}
	
	for (i = 0; i < N_CONS+N_PROD; i++)
		wait(NULL);
	close(s);
	return 0;
}