#include "lock.h"

LOCAL void clear_lock(int lid);

SYSCALL lcreate()
{
	int lid = -1;
	int i;
	STATWORD ps;
	disable(ps);
	for (i = 0; i < NLOCKS; i++){
		if (locktab[i].state == LFREE){
			lid = i;
			nlocks++;
			break;
		}
	}
	if (ISBADLID(lid)){
		restore(ps);
		return SYSERR;
	}
	clear_lock(lid);
	restore(ps);
	return LID2LDESC(lid);
}

LOCAL void clear_lock(int lid)
{
	lock_t * lock = &(locktab[lid]);
	lock->state = UNLOCKED;
	lock->type = DELETED;
	lock->age++;
	lock->nreaders = 0;
	lock->nwriters = 0;
}
