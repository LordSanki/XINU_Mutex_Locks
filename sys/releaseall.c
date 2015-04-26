#include "lock.h"
#include "proc.h"
#include "q.h"

SYSCALL releaseall(int nlocks, ...)
{
	int *args = &nlocks;
	int ldesc, lid;
	int ret = OK;
	int next;
	lock_t *lock;
	STATWORD ps;
	disable(ps);
	for (; nlocks > 0; nlocks--){
		ldesc = *(--args);
		lid = LDESC2LID(ldesc);
		if (ISBADLID(lid)){
			ret = SYSERR;
			continue;
		}
		lock = &locktab[lid];
		if ((lock->state != LOCKED) || 
			(lock->age != LDESC2AGE(ldesc)) ||
			(lock->procs[currpid].lstate != LOCKED)
			) {
			lock->procs[currpid].lstate = UNLOCKED;
			ret = SYSERR;
			continue;
		}
		/* Updating readers & writers count*/
		if (lock->procs[currpid].ltype == READ)
			lock->nreaders--;
		else
			lock->nwriters--;
		
		/* actual unlocking*/
		lock->procs[currpid].lstate = UNLOCKED;
		if ((lock->nreaders + lock->nwriters) == 0){
			lock->state = UNLOCKED;
		}

		/* Now trying to grant read locks if possible */
		next = q[lock->tail].qprev;
		while (next != lock->head){
			if (lock->procs[next].ltype == READ){
				grant_lock(next, lock, TRUE);
			}
			else{
				break;
			}
			next = q[next].qprev;
		}
		/* No read locks were given trying to give write lock now*/
		if (lock->state == UNLOCKED){
			if (nonempty(lock->head)){
				int pid = getlast(lock->tail);
				grant_lock(pid, lock, TRUE);
				//assert(lock->procs[pid].ltype == WRITE);
			}
		}
	}
	return ret;
}
