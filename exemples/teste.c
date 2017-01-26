#include <stdio.h>
#include <unistd.h>
#include <semaforo.h>


int main(void)
{
	int s1;
	int s2;
	int pid;
	int i;	
	FILE * f;
	printf("Inicio\n");
	s1 = sem_create(0);
	s2 = sem_create(1);
	f = fopen("saida", "w");
	sleep(1);
	pid = fork();
	if (pid < 0) return 1;

	if (pid == 0) 
	{
		printf("Deu fork()\n");
		for (i = 0; i < 5; i++) 
		{
			sem_p(s2);
			write(fileno(f), "Pro", 3);
			sleep(1);
			write(fileno(f), "duz\n", 4);
			sleep(1);
			sem_v(s1);	
		}
		fclose(f);
	} else {
		printf("Deu fork()\n");
		for (i = 0; i < 5; i++) 
		{
			sem_p(s1);
			write(fileno(f), "Cons", 4);
			sleep(1);
			write(fileno(f), "ome\n", 4);
			sleep(1);
			sem_v(s2);
		}
		
		fclose(f);
	}
	return 0; 
}
