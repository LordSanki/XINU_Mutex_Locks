#ifndef __LOCK_H__
#define __LOCK_H__

#include "kernel.h"
#include "proc.h"
#define NLOCKS 50

#define LFREE 0
#define UNLOCKED 1
#define LOCKED 2
#define WAITING 3

//#define DELETED 51
#define READ 1
#define WRITE 2

// bit 0-1 are lock type
// bit 3-2 are lock state

typedef struct {
	unsigned char ltype;
	unsigned char lstate;
	unsigned int lage;
}procdesc_t;

#define LID2LDESC(ID) ((locktab[ID].age << 6)|ID)
#define LDESC2LID(DESC) (DESC & 0x3F)
#define LDESC2AGE(DESC) (DESC >> 6)
#define CREATELDESC(ID, AGE) ((AGE<<6) | ID)
#define ISBADLID(ID) (ID < 0 || ID >= NLOCKS)

//#define PD2LTYPE(PD) (PD & 0x3)
//#define PD2LSTATE(PD) ((PD>>2) & 0x3)
///#define LTYPE2PD(LT, PD) ((PD&0xFC) | LT)
//#define LSTATE2PD(LS, PD) ((PD&0xF3) | (LS<<2))

typedef struct {
	int type;
	int state;
	unsigned int age;
	procdesc_t procs[NPROC];
	int nreaders;
	int nwriters;
	int tail;
	int head;
}lock_t;

extern lock_t locktab[];
extern int nlocks;
extern SYSCALL lcreate();
extern SYSCALL ldelete(int ldesc);
extern SYSCALL releaseall(int nlocks, ...);
extern SYSCALL lock(int ldesc, int type, int prio);
extern SYSCALL linit();
extern void grant_lock(lock_t * lock, int pid, int ready_proc);

#endif //__LOCK_H__
