#include <lib.h>
#define sem_destroy _sem_destroy
#define SEM_DESTROY 66
PUBLIC int sem_destroy(int semid)
{
   message m;
   m.m1_i1 = semid;
   return _syscall(MM, SEM_DESTROY, &m);
}

