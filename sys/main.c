/* user.c - main */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <lock.h>

void halt();

void reader(char c, int lck, int prio, int setupTime, int holdTime){
	int i,j=0,retval;
	for(j=0; j<5; j++) {
		sleep(setupTime);
		if(OK != (retval = lock(lck,READ,prio) ) ) {
			kprintf("Reader: %c%c%c Could not lock %d. Return: %d !\n",c,c,c,lck,retval);
			return;
		}
		for(i=0; i<holdTime*10; i++) {
			kprintf("Reader: %c%c%c%c%c%d\n",c,c,c,c,c,j);
			sleep1000(50);
		}
		if(OK != (retval = releaseall(1,lck) ) ) {
			kprintf("Reader: %c%c%c Could not release lock %d. Return: %d !\n",c,c,c,lck,retval);
			return;
		}
	}
}
void writer(char c, int lck, int prio, int setupTime, int holdTime){
	int i,j=0,retval;
	for(j=0; j<5; j++) {
		sleep(setupTime);
		if(OK != (retval = lock(lck,WRITE,prio) ) ) {
			kprintf("Writer: %c%c%c Could not lock %d. Return: %d !\n",c,c,c,lck,retval);
			return;
		}
		for(i=0; i<holdTime*10; i++) {
			kprintf("Writer: %c%c%c%c%c%d\n",c,c,c,c,c,j);
			sleep1000(50);
		}
		sleep(holdTime);
		if(OK != (retval = releaseall(1,lck) ) ) {
			kprintf("Writer: %c%c%c Could not release lock %d. Return: %d !\n",c,c,c,lck,retval);
			return;
		}
	}
}

void samePriorityInterleave()
{
	int i,j=0,retval;
	int lck1;
	/*create lock*/
	kprintf("Launcher %s starting\n",__func__);
	lck1 = lcreate();
	if(lck1 < 0) {
		kprintf("Launcher %s: Unable to get lock. Return: %d. Test failed\n",__func__,lck1);
		return;
	}
	kprintf("\n\nTest: %s\n",__func__);
	resume(create(writer,2000,20,"writerA",5,'A', lck1, 24, 1, 3));
	resume(create(writer,2000,20,"writerB",5,'B', lck1, 24, 2, 4));
	resume(create(writer,2000,20,"writerC",5,'C', lck1, 24, 1, 4));
	resume(create(writer,2000,20,"writerD",5,'D', lck1, 24, 3, 4));
	resume(create(reader,2000,20,"readerE",5,'E', lck1, 24, 2, 5));
	sleep(1);
	resume(create(reader,2000,20,"readerF",5,'F', lck1, 24, 2, 5));
	sleep(1);
	resume(create(reader,2000,20,"readerG",5,'G', lck1, 24, 2, 5));
	sleep(1);
	resume(create(reader,2000,20,"readerH",5,'H', lck1, 24, 2, 5));
	sleep(1);
	resume(create(reader,2000,20,"readerI",5,'I', lck1, 24, 2, 5));
	sleep(1);
	resume(create(reader,2000,20,"readerJ",5,'J', lck1, 24, 2, 5));
	while (1) {
		kprintf("Looping in launcher %s\n", __func__);
		sleep(2);
		j++;
		if(j == 10) break;
	}
	sleep(150);
	if(OK != (retval = ldelete(lck1) ) ) {
		kprintf("Launcher %s: Delete on the lock %d failed with code %d\n",__func__,lck1,retval);
		return;
	}
	kprintf("Launcher %s completed.\n",__func__);
}


void multiPriorityInterleave() {
	int i,j=0;
	int lck1,retval;
	/*create lock*/
	kprintf("Launcher %s starting\n",__func__);
	lck1 = lcreate();
	if(lck1 < 0) {
		kprintf("Launcher %s: Unable to get lock. Return: %d. Test failed\n",__func__,lck1);
		return;
	}
	kprintf("\n\nTest: %s\n",__func__);
	resume(create(writer,2000,20,"writerK",5,'K', lck1, 24, 2, 3));
	resume(create(reader,2000,20,"readerL",5,'L', lck1, 30, 9, 5));
	resume(create(reader,2000,20,"readerM",5,'M', lck1, 30, 9, 5));
	resume(create(reader,2000,20,"readerN",5,'N', lck1, 22, 1, 5));
	resume(create(reader,2000,20,"readerO",5,'O', lck1, 24, 2, 5));
	resume(create(reader,2000,20,"readerP",5,'P', lck1, 26, 3, 5));
	resume(create(reader,2000,20,"readerQ",5,'Q', lck1, 28, 4, 5));
	sleep(1);
	resume(create(reader,2000,20,"readerR",5,'R', lck1, 24, 2, 5));
	while (1) {
		kprintf("Looping in launcher %s\n", __func__);
		sleep(2);
		j++;
		if(j == 10) break;
	}
	sleep(150);
	if(OK != (retval = ldelete(lck1) ) ) {
		kprintf("Launcher %s: Delete on the lock %d failed with code %d\n",__func__,lck1,retval);
		return;
	}
	kprintf("Launcher %s completed.\n",__func__);
}
void getAllLocks(char c, int *lockArr, int lockType, int prio, int setupTime, int holdTime){
	int i,j=0,retval;
	while(1) {
		sleep(setupTime);
		for(i=0; i<NLOCKS; i++) {
//			printf("%c %d %d %d %d %d\n",c,lockArr[0],lockArr[1],lockArr[2],lockArr[3],lockArr[4]);
			retval = lock(lockArr[i], lockType, prio);
			if(retval != OK) {
				kprintf("StressTest op-%d: %c%c%c%c%c Lock %d failed : %d\n",lockType,c,c,c,c,c, lockArr[i], retval);
				return;
			}
		}
//		kprintf("%c%c%c%c%c\n",c,c,c,c,c);
		for(i=0; i<holdTime*10; i++) {
			kprintf("StressTest op-%d: %c%c%c%c%c\n",lockType,c,c,c,c,c);
			sleep1000(50);
		}
		for(i=0; i<NLOCKS; i+=2 ) {
			retval = releaseall(2, lockArr[i], lockArr[i+1]);
			if(retval != OK) {
				kprintf("StressTest op-%d: %c%c%c%c%c Release %d failed : %d\n",lockType,c,c,c,c,c,lockArr[i],retval);
				return;
			}
		}
		j++;
		if(j == 10) break;
	}
}
void stressTest(){
	int i,j;
	int ldes[NLOCKS];
	kprintf("Launcher %s starting\n",__func__);
	for(i=0; i<NLOCKS; i++) {
		ldes[i] = lcreate();
		if(ldes[i] < 0) {
			kprintf("Launcher %s: Unable to get %dth lock. Return: %d. Test failed\n",__func__,i,ldes[i]);
			return;
		}
	}

	resume(create(getAllLocks,2000,20,"procStresstestRdW",6,'W', ldes,READ, 10, 1, 3));
	resume(create(getAllLocks,2000,20,"procStresstestRdX",6,'X', ldes,READ, 10, 1, 3));
	resume(create(getAllLocks,2000,20,"procStresstestWrY",6,'Y', ldes,WRITE, 10, 1, 3));
	resume(create(getAllLocks,2000,20,"procStresstestWrZ",6,'Z', ldes,WRITE, 10, 1, 3));

	sleep(20);
	kprintf("Launcher %s: Deleting all locks. All children should exit or error out now, depending on their status.\n",__func__);
	for(i=0; i<NLOCKS; i++) {
		int retval;
		if(OK != (retval = ldelete(ldes[i]) ) ) {
			kprintf("Launcher %s: Delete on the lock %d failed with code %d\n",__func__,ldes[i],retval);
		}
		if(i==10) sleep(3);
	}
	sleep(5);
	kprintf("Launcher %s completed. Check messages till now for any failures.\n",__func__);
}

void killAndDeleteTest() {
	int i,j,ldes,retval;
	int pid1,pid2,pid3,pid4,pid5;

	kprintf("Launcher %s starting\n",__func__);
	ldes = lcreate();
	if(ldes < 0) {
		kprintf("Launcher %s: Unable to get lock. Return: %d. Test failed\n",__func__,ldes);
		return;
	}
	if(OK != (retval = lock(ldes,WRITE,50) ) ) {
		kprintf("Launcher %s: Unable to open lock %d. Return: %d FAIL!\n",__func__, ldes,retval);
		return;
	}
	kprintf("Launcher %s: Holding lock %d . 1st process S should not be able to start printing, even after resuming.\n",__func__, ldes);
	pid1 = create(writer,2000,20,"wrS",5,'S', ldes, 30, 1, 9);
	pid2 = create(writer,2000,20,"wrT",5,'T', ldes, 30, 1, 9);
	pid3 = create(writer,2000,20,"wrU",5,'U', ldes, 30, 1, 5);
	pid4 = create(writer,2000,20,"wrV",5,'V', ldes, 30, 1, 5);
	resume(pid1);
	sleep(1);
	resume(pid2);
	sleep(1);
	resume(pid3);
	sleep(1);
	resume(pid4);
	kill(pid1);
	if(OK != (retval = releaseall(1,ldes) ) ) {
		kprintf("Launcher %s: Could not release lock %d. Return: %d !\n",__func__,ldes,retval);
		return;
	}
	sleep(1);
	kprintf("Launcher %s: Waiting kill done. Now the 2nd process T should have got the lock and started printing. Will kill it now.\n",__func__, ldes,retval);
	kill(pid2);
	sleep(1);
	kprintf("Launcher %s: Holding kill done. Now the 3rd process U should have got the lock and started printing. Will delete lock now.\n",__func__, ldes,retval);
	if(OK != (retval = ldelete(ldes) ) ) {
		kprintf("Launcher %s: Delete on the lock %d failed with code %d\n",__func__,ldes,retval);
		return;
	}
	kprintf("Launcher %s: Live and Waiting delete done. 3rd process U should fail during release with -1, and 4th process V should fail with return code of -6.\n",__func__,ldes,retval);
	sleep(12);
	kprintf("Launcher %s completed. Check messages till now for any failures.\n",__func__);
}

void tooManyLocksAndSomeOtherFuncTests() {
	int i,j,retval;
	int ldes[NLOCKS];

	kprintf("Launcher %s starting\n",__func__);
	for(i=0; i<NLOCKS; i++) {
		ldes[i] = lcreate();
		if(ldes[i] < 0) {
			kprintf("Launcher %s: Unable to get lock. Return: %d. Test failed!\n",__func__,ldes[i]);
			return;
		}
	}
	j = lcreate();
	if(j != SYSERR) {
		kprintf("Launcher %s: Too many locks getting created. This one should have failed. Return: %d. Test failed!\n",__func__,j);
		return;
	}
	if(SYSERR != (retval = releaseall(1,ldes[30]) ) ) {
		kprintf("Launcher %s: The release on lock %d should have failed, since we never locked it. Return: %d. Test failed!\n",__func__,ldes[30],retval);
		return;
	}
	if(OK != (retval = ldelete(ldes[30]) ) ) {
		kprintf("Launcher %s: Delete on the lock %d failed with code %d\n",__func__,ldes[30],retval);
		return;
	}
	if(SYSERR != (retval = releaseall(1,ldes[30]) ) ) {
		kprintf("Launcher %s: The release on lock %d should have failed, since we've already deleted the lock. Return: %d. Test failed!\n",__func__,ldes[30],retval);
		return;
	}
	j = lcreate();
	if(j < 0) {
		kprintf("Launcher %s: A space for one lock was created. We should have got a lock now. Return: %d. Test failed!\n",__func__,j);
		return;
	}
	ldes[30]=j;
	for(i=0; i<NLOCKS; i++) {
		int retval;
		if(OK != (retval = ldelete(ldes[i]) ) ) {
			kprintf("Launcher %s: Delete on the lock %d failed with code %d\n",__func__,ldes[i],retval);
		}
	}
	kprintf("Launcher %s completed. Check messages till now for any failures.\n",__func__);
}

int main() {
	//killAndDeleteTest();
	kprintf("\n=====================================================================\n");
	tooManyLocksAndSomeOtherFuncTests();
	kprintf("\n=====================================================================\n");
	//samePriorityInterleave();
	kprintf("\n=====================================================================\n");
	//multiPriorityInterleave();
	kprintf("\n=====================================================================\n");
	//stressTest();
	kprintf("\n=====================================================================\n");
	//stressTest();
	kprintf("\n=====================================================================\n");
	//stressTest();
	kprintf("\n=====================================================================\n");
	kprintf("\n=========================All tests done==============================\n");
}
