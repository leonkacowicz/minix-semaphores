#include <lib.h>
#define sem_v _sem_v
#define SEM_V 70
PUBLIC int sem_v(int semid)
{
   message m;
   m.m1_i1 = semid;
   return _syscall(MM, SEM_V, &m);
}

