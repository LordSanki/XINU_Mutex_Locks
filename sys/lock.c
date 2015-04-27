#include "lock.h"
#include "proc.h"
#include "q.h"
#include "sleep.h"

LOCAL int start_wait(int pid, int prio, int age, lock_t *lock, STATWORD ps);
LOCAL int enqueue_lock(int pid, lock_t *lck, int key);
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
	proctab[pid].pstate = PRLOCK;
	lock->procs[currpid].lstate = WAITING;
	lock->procs[currpid].ltime = clktime;
	enqueue_lock(pid, lock, prio);
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
	lock->procs[pid].ltime = clktime;
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

LOCAL int enqueue_lock(int pid, lock_t *lck, int key)
{
	int	next;			/* runs through list		*/
	int	prev;
	int lead;

	next = q[lck->head].qnext;
	/* tail has maxint as key	*/
	while (key > q[next].qkey){
		next = q[next].qnext;
	}
	//prev = q[next].qprev;
	if (key == q[next].qkey){
		lead = next;
		while (q[lead].qkey == q[q[lead].qnext].qkey){
			lead = q[lead].qnext;
		}
		// if current lock is write
		if (lck->procs[pid].ltype == WRITE){
			// if lead is read
			if (lck->procs[lead].ltype == READ){
				// check to see if time is equal
				if (lck->procs[pid].ltime == lck->procs[lead].ltime){
					// if yes then curr lock becomes lead
					next = q[lead].qnext;
				}
			}
			// if lead is write
			if (lck->procs[lead].ltype == WRITE){
				// find if there is any read lock with greater w8 time
			}
		}
		/*current lock is read*/
		else{
			// if lead is READ then queing behind it
			if (lck->procs[lead].ltype == READ){
				next = lead;
			}
			// else if it is write the stay where we are
		}
	}
	q[pid].qnext = next;
	q[pid].qprev = prev = q[next].qprev;
	q[pid].qkey = key;
	q[prev].qnext = pid;
	q[next].qprev = pid;
	return(OK);
}


