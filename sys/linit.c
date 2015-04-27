#include "lock.h"

lock_t locktab[NLOCKS];
int nlocks;

SYSCALL linit()
{
	int i,j;
	lock_t *lock;
	for (i = 0; i < NLOCKS; i++){
		lock = &locktab[i];
		lock->type = DELETED;
		lock->state = LFREE;
		lock->age = 0;
		lock->head = newqueue();
		lock->tail = 1 + lock->head;
    for(j = 0; j < NPROC; j++){
      lock->procs[j].lstate = UNLOCKED;
      lock->procs[j].ltype = DELETED;
      lock->procs[j].lage = 0;
    }
	}
	nlocks = 0;
  return OK;
}
