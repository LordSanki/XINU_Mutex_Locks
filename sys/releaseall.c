#include "lock.h"
#include "proc.h"
#include "q.h"


SYSCALL releaseall(int nlocks, ...)
{
	int *args = &nlocks;
	int ldesc;
	int ret = OK;
	STATWORD ps;
	disable(ps);
	for (; nlocks > 0; nlocks--){
		ldesc = *(++args);
    if(OK != release_lock(ldesc, currpid))
      ret = SYSERR;
	}
  restore(ps);
	return ret;
}

int release_lock(int ldesc, int pid)
{
	int next;
	lock_t *lock;

  int lid = LDESC2LID(ldesc);
  if (ISBADLID(lid)){
    return SYSERR;
  }
  lock = &locktab[lid];
  if ((lock->state != LOCKED) || 
      (lock->age != LDESC2AGE(ldesc)) ||
      (lock->procs[pid].lstate != LOCKED)
     ) {
    lock->procs[pid].lstate = UNLOCKED;
    return SYSERR;
  }
  //kprintf("Readers %d,  Writers %d  \n",lock->nreaders, lock->nwriters);
  /* Updating readers & writers count*/
  if (lock->procs[pid].ltype == READ)
    lock->nreaders--;
  else
    lock->nwriters--;

  /* actual unlocking*/
  lock->procs[pid].lstate = UNLOCKED;
  if ((lock->nreaders + lock->nwriters) == 0){
    lock->state = UNLOCKED;
  }

  /* Now trying to grant read locks if possible */
  next = q[lock->tail].qprev;
  while (next != lock->head){
    if (lock->procs[next].ltype == READ){
      grant_lock(lock, next, TRUE);
    }
    else{
      break;
    }
    //kprintf("Try Readlocks in release\n");
    next = q[next].qprev;
  }
  /* No read locks were given trying to give write lock now*/
  if (lock->state == UNLOCKED){
    //kprintf("Try Write Locks in release\n");
    if (nonempty(lock->head)){
      int pid = getlast(lock->tail);
      grant_lock(lock, pid, TRUE);
      //kprintf("Write lock granted\n");
      //assert(lock->procs[pid].ltype == WRITE);
    }
  }
  return OK;
}

