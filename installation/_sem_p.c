#include <lib.h>
#define sem_p _sem_p
#define SEM_P 69
PUBLIC int sem_p(int semid)
{
   message m;
   m.m1_i1 = semid;
   return _syscall(MM, SEM_P, &m);
}

