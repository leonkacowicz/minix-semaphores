
#include <stdio.h>

#include <semaforo.h>


int main(void)
{
	int i;
	for (i = 0; i < 256; i++)
		if (!sem_destroy(i)) printf("Semaforo %d destruido com sucesso.\n", i);
	return 0;
		
}