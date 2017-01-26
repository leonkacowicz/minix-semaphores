#include <lib.h>
#define sem_create _sem_create
#define SEM_CREATE 64
PUBLIC int sem_create(int val)
{
   message m;
   m.m1_i1 = val;
   return _syscall(MM, SEM_CREATE, &m);
}

