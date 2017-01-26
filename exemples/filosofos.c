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

#define ITER 10

int s;
int sem_garfos[5];


int menor(int filosofo)
{
	int g1, g2;
	g1 = filosofo -1;
	g2 = filosofo % 5;
	
	if (g1 < g2) return g1;
	return g2;
}

int maior(int filosofo)
{
	int g1, g2;
	g1 = filosofo -1;
	g2 = filosofo % 5;
	
	if (g1 > g2) return g1;
	return g2;
}


void filosofo(int id)
{
	int i;
	char msg1[50];
	char msg2[50];
	sprintf(msg1, "%2d Comecou a comer\n", id);
	sprintf(msg2, "%2d Terminou de comer\n", id);
	for (i = 0; i < ITER; i++)
	{
		SEM__P(sem_garfos[menor(id)]);
		SEM__P(sem_garfos[maior(id)]);
		
		write(s, msg1, 19);
		printf("%2d Comendo\n", id);
		
		/* demora comendo para ser preemptado */
		sleep(1);
		
		write(s, msg2, 21);
		SEM__V(sem_garfos[maior(id)]);
		SEM__V(sem_garfos[menor(id)]);
		
		printf("                                      %2d pensando.\n", id);
		sleep(1);
		
	}
}

int main(int argc, char * argv[])
{
	int i;
	int pid;
	int fd;

	#if COM_SEMAFOROS
	for (i = 0; i < 5; i++)
		sem_garfos[i] = SEM__CREATE(1);
	#endif
	
	s = open("filosofos.saida",  O_CREAT | O_WRONLY);
	for (i = 1; i <= 5; i++)
	{
		pid = fork();
		if (pid < 0) return 1;
		if (pid == 0)
		{
			printf("Criando filosofo %d\n", i);
			filosofo(i);
			return 0;
		}
	}
	
	for (i = 0; i < 5; i++)
		wait(NULL);
	close(s);
	
	for (i = 0; i < 5; i++)
		SEM__DESTROY(sem_garfos[i]);
	return 0;
}


