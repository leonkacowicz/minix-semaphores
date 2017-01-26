#include <stdio.h>
#include <unistd.h>
#include <semaforo.h>

int main(void)
{
	int s;
	printf("Meu pid eh %d\n", getpid());
	s = sem_create(1);
	sem_p(s);
	sem_p(s);
	return sem_destroy(s);
}

