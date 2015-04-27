#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <lock.h>


/* test1.c
 * This test program creates three processes, two readers and a writer at
 * priority 20.  The main process also has priority 20.
 * All readers and writers have the same priority.
 */

int reader(char, int, int);
int writer(char, int, int);
int del_test1(char c, int lck);
int del_test2(char c, int lck);
int del_test3(char c, int lck);
int peak_load_test(char c);

int reader1, reader2;
int writer1;
int lck1;

int main()
{
  int i;
  /*create lock*/

#if 1
  kprintf("\n\nTEST1: Expect RRRRRRWRWRWR....\n");
  lck1 = lcreate();
  resume(reader1 = create(reader,2000,20,"reader1",3,'1', lck1, 100));
  resume(reader2 = create(reader,2000,20,"reader2",3,'2', lck1, 100));
  resume(writer1 = create(writer,2000,20,"writer1",3,'1', lck1, 94));
  sleep(10);
  kill (reader1);
  kill (reader2);
  kill (writer1);
  kprintf(" Locking %d\n", lock(lck1,READ,100));
  releaseall(1,lck1);
  ldelete(lck1);
  kprintf("\nTest Done\n");
#endif

#if 1
  kprintf("\n\n Test2: Deletion test\n\n");
  lck1 = lcreate();
  resume(reader1 = create(del_test1, 20000, 20, "Del1", 3, '1', lck1)); 
  resume(reader2 = create(del_test2, 20000, 20, "Del1", 3, '2', lck1)); 
  sleep(1);
  resume(writer1 = create(del_test3, 20000, 20, "Del1", 3, '3', lck1));
  sleep(4);
  kill (reader1);
  kill (reader2);
  kill (writer1);
  ldelete(lck1);
  kprintf("\nTest Done\n");
#endif
#if 1
  kprintf("\n\n Test3: Peak Load test\n\n");
  resume(reader1 = create(peak_load_test, 20000, 20, "Del1", 3, '1')); 
  sleep(5);
  kprintf("\nTest Done\n");
#endif
  while (1) {
    sleep(1);
  }
}


int reader(char c, int lck, int prio){
  int i;

  while(1) {
    lock(lck, READ, prio);
    kprintf("Reader(%c)\n",c);
    sleep(1);
    prio -= 2;
    releaseall(1, lck);
  }
}

int writer(char c, int lck, int prio){
  int i;

  while(1) {
    lock(lck, WRITE, prio);
    kprintf("Writer(%c)\n",c);
    sleep(1);
    releaseall(1, lck);
  }
}

int del_test1(char c, int lck){
  while(1){
    kprintf("Proc %c Locking %d\n",c, lock(lck, WRITE, 10));
    sleep(10);
    releaseall(1,lck);
  }
}
int del_test2(char c, int lck){
  while(1){
    kprintf("Proc %c Locking %d\n",c, lock(lck, WRITE, 10));
    sleep(1);
    releaseall(1,lck);
  }
}
int del_test3(char c, int lck){
  sleep(2);
  kprintf("Proc %c deleting %d\n", c, ldelete(lck));
  while(1){
  }
}

int peak_load_test(char c){
  int lck[50];
  int i;
  for(i=0; i<50; i++){
    lck[i] = lcreate();
    if(OK != lock(lck[i], WRITE, 10)){
      kprintf("Test Failed!! \n");
      return;
    }
  }
  for(i=0; i<50; i++){
    if(OK == lock(lck[i], WRITE, 10)){
      kprintf("Test Failed!! \n");
      return;
    }
  }
  for(i=0; i<50; i += 5){
    if(OK != releaseall(5, lck[i], lck[i+1], lck[i+2], lck[i+3], lck[i+4])){
      kprintf("Test Failed!! \n");
      return;
    }
  }
  for(i=0; i<50; i++){
    if(OK != ldelete(lck[i])){
      kprintf("Test Failed!! \n");
      return;
    }
  }
  lck[0] = lcreate();
  if(OK != lock(lck[0], READ, 10)){
       kprintf("Test Failed!! \n");
       return;
  }
  releaseall(1, lck[0]);
  ldelete(lck[0]);
}

