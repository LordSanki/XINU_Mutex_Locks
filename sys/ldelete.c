#include "lock.h"
#include "proc.h"
#include "q.h"

SYSCALL ldelete(int ldesc)
{
	STATWORD ps;
	int lid = LDESC2LID(ldesc);
	lock_t *lock;
	int pid;
	if (ISBADLID(lid))
		return SYSERR;
	lock = &locktab[LDESC2LID(ldesc)];
	if (lock->age != LDESC2AGE(ldesc) || lock->state == LFREE)
		return SYSERR;
	disable(ps);
	lock->state = LFREE;
	lock->type = DELETED;
	while (nonempty(lock->head)){
		pid = getfirst(lock->head);
    lock->procs[pid].lstate = UNLOCKED;
		ready(pid, RESCHNO);
	}
  restore(ps);
	return OK;
}
