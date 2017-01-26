


#include "pm.h"
#include <minix/callnr.h>
#include <signal.h>
#include <sys/svrctl.h>
#include <sys/resource.h>
#include <minix/com.h>
#include <string.h>
#include "mproc.h"
#include "param.h"

/*
 * Codigo do Semaforo
 */
#define NUM_MAX_PROCESSOS 256
#define NUM_MAX_SEMAFOROS 256

#define PRINT

typedef struct
{
	unsigned int sem_usado;
	unsigned int valor;
	unsigned int num_processos_bloq;
	unsigned int lista_processos_bloq[ NUM_MAX_PROCESSOS ];
} sem;

sem semaforos[ NUM_MAX_SEMAFOROS ];
int sem_inicializados = 0;

void inic_semaforos( void )
{
	int i;
	if (sem_inicializados) return;
	for (i = 0; i < NUM_MAX_SEMAFOROS; i++)
	{
		semaforos[i].sem_usado = 0;
	}
	sem_inicializados = 1;
}

int tem_erro_no_sem_id( int sem_id )
{
	if ((sem_id >= NUM_MAX_SEMAFOROS) || (sem_id < 0)) return -1;
	if (semaforos[sem_id].sem_usado == 0) return -1;
	return 0;
}

int do_sem_create( void )
{

	int i;
	int sem_valor;

	PRINT("Criando semaforo..\n");
	
	sem_valor = m_in.m1_i1;
	inic_semaforos();   
	/* procura um semaforo livre */
	for (i = 0; i < NUM_MAX_SEMAFOROS; i++)
	{
		if (!semaforos[i].sem_usado) 
		{
			semaforos[i].sem_usado = 1;
			semaforos[i].num_processos_bloq = 0;
			
			/* coloca o valor inicial passado na mensagem */
			semaforos[i].valor = sem_valor;
         	
			PRINT("Semaforo criado com id = %d\n", i);
   
			return i;
		}
	}	
	
	/* nao encontrou nenhum livre, retorna erro */
	return -1;

}

int do_sem_destroy( void )
{

	int sem_id;
	int i;
	
	/* pega o id do semaforo da msg enviada */
	sem_id = m_in.m1_i1;
   
	if (sem_inicializados == 0)
		return -1;
	
	if (tem_erro_no_sem_id(sem_id))
		return -1;
	
   	/* marca o semaforo como livre para uso */
	semaforos[sem_id].sem_usado = 0;
	for (i = 0; i < semaforos[sem_id].num_processos_bloq; i++)
	{
		/* ao destruir o semaforo, desbloqueia os processos
		 * bloqueados por ele
		 */
		setreply(semaforos[sem_id].lista_processos_bloq[i], 0);
	}
	PRINT("Destruindo semaforo #%d\n", sem_id);	
	/* retorna sem erros */
	return 0;
}
int do_sem_p( void )
{
	int sem_id, i, spid;
	if (sem_inicializados == 0)
		return -1;
	
	sem_id = m_in.m1_i1;
	if (tem_erro_no_sem_id(sem_id))
		return -1;
	
	spid = m_in.m_source;
   	PRINT("Decrementando Semaforo #%d (enviado por #%d)\n", sem_id, spid);
	
	if (semaforos[sem_id].valor == 0)
	{
		i = semaforos[sem_id].num_processos_bloq;
		if (i == NUM_MAX_PROCESSOS) 
			return -1; /* atingiu o numero maximo de processos permitidos*/

		semaforos[sem_id].num_processos_bloq++;

		semaforos[sem_id].lista_processos_bloq[i] = spid;
		
      
		PRINT("Bloqueando o processo #%d\n", spid);
		/* neste ponto o processo ja esta bloqueado:
		 * apenas deixamos ele sem resposta para
		 * mante-lo bloqueado, retornando SUSPEND */
      
		return SUSPEND;
	} else {
		semaforos[sem_id].valor--;
		return 0;
	}
}
int do_sem_v( void )
{
	int sem_id;
	int desbloq;
	int i;
	
	if (sem_inicializados == 0)
		return -1;
	
   
	sem_id = m_in.m1_i1;
	
   
	if (tem_erro_no_sem_id(sem_id))
		return -1;
	PRINT("Incrementando Semaforo #%d\n", sem_id);
	if (semaforos[sem_id].num_processos_bloq == 0)
	{
		semaforos[sem_id].valor++;
		return 0;
	} else {
		desbloq = semaforos[sem_id].lista_processos_bloq[0];
		for (i = 0; i < semaforos[sem_id].num_processos_bloq - 1; i++)
		{
			semaforos[sem_id].lista_processos_bloq[i] =
				semaforos[sem_id].lista_processos_bloq[i + 1];
		}
		semaforos[sem_id].num_processos_bloq--;

		PRINT("Desbloqueando o processo #%d que estava bloqueado\n",
				desbloq);
		/* Seta o valor da mensagem de resposta (para desbloquear o 
	      	* processos q estava bloqueado esperando essa resposta) 
      		*/
      		setreply(desbloq, 0);
      
	}
	
	return 0;
}



