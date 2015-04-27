#include "lock.h"
#include "proc.h"
#include "q.h"

LOCAL int start_wait(int pid, int prio, int age, lock_t *lock, STATWORD ps);

SYSCALL lock(int ldesc, int type, int prio)
{
	int lid = LDESC2LID(ldesc);
	lock_t *lock;
	STATWORD ps;
	/* error checking */
	if (ISBADLID(lid))
		return SYSERR;

	lock = &locktab[lid];

	/* Lock is uninitialized error */
	if (lock->state == LFREE)
		return SYSERR;
	/* Stale lock error */
	if (lock->age != LDESC2AGE(ldesc))
		return SYSERR;

	disable(ps);

	/* Saving the type of lock requested by proc */
	lock->procs[currpid].ltype = type;

	/* locking successfull */
	if (lock->state == UNLOCKED){
		grant_lock(lock, currpid, FALSE);
		restore(ps);
		return OK;
	}

	/* mutex is already locked */

	//FIXME: some more logic for locking need to go here
	/* lock type is read and request is also read.
		We can grant lock if there is no higher prio Write lock w8ing */
	if ((lock->type == READ) && (type == READ)){
		int next = q[lock->tail].qprev;
		int wait = FALSE;
		while (next != lock->head){
			if (q[next].qkey > prio){
				if (lock->procs[next].ltype == WRITE){
					wait = TRUE;
					break;
				}
			}
			else{
				break;
			}
			next = q[next].qprev;
		}
		/* we found a writer with higher prio */
		/* need to w8 in que */
		if (wait == TRUE){
			return start_wait(currpid, prio, LDESC2AGE(ldesc), lock, ps);
		}
		/* there is no writer with higher prio we can get lock*/
		else{
			grant_lock(lock, currpid, FALSE);
			restore(ps);
			return OK;
		}
	}

	/*  we need to w8 in queue for lock*/
	return start_wait(currpid, prio, LDESC2AGE(ldesc), lock, ps);
}

LOCAL int start_wait(int pid, int prio, int age, lock_t *lock, STATWORD ps)
{
	dequeue(pid);
	insert(pid, lock->head, prio);
	proctab[pid].pstate = PRLOCK;
	lock->procs[currpid].lstate = WAITING;
	restore(ps);
	/* W8ing starts*/
	resched();
	/* w8ing ends if lock was deleted or we got the lock*/
	if((lock->state == LFREE) || (lock->age != age))
		return DELETED;
	else
		return OK;
}

void grant_lock(lock_t * lock, int pid, int ready_proc)
{
	lock->procs[pid].lstate = LOCKED;
	lock->state = LOCKED;
	lock->type = lock->procs[pid].ltype;
	if (lock->type == READ)
		lock->nreaders++;
	else
		lock->nwriters++;
	lock->procs[pid].lage = lock->age;
	if (ready_proc == TRUE){
		dequeue(pid);
		ready(pid, RESCHNO);
	}
}
