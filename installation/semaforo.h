
#define SEM_CREATE 64
#define SEM_DESTROY 66
#define SEM_P 69
#define SEM_V 70

#define sem_create _sem_create
#define sem_destroy _sem_destroy
#define sem_p _sem_p
#define sem_v _sem_v

int sem_create(int val);
int sem_destroy(int semid);
int sem_p(int semid);
int sem_v(int semid);
