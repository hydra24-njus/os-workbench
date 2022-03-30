#include <common.h>

void test1(int tid) {
  for (int i = 1; i <= 2000; i++) {
    pmm->alloc(5000 + (i * 1000) % 5000);
    // if (i % 5 == 0) pmm->free(loc);
  }
  printf("end1\n");
}

void test2(int tid) {
  printf("start2\n");
  void* loc;
  for (int i = 1; i <= 10000; i++) {
    size_t a = (rand() % 2) ? rand() % 2000 + 1000 : rand() % 100 + 50;
    loc = pmm->alloc(a);
    printf("loc=%p\n",loc);
    if(loc==NULL)assert(0);
    //if (i % 5 == 0) pmm->free(loc);
  }
  printf("end2\n");
}

void test3(int tid) {
  void* loc;
  for (int i = 1; i <= 10000; i++) {
    loc = pmm->alloc(5000 + (i * 1000) % 5000);
    pmm->free(loc);
  }
  printf("end3\n");
}

void test4(int tid) {
  void* loc;
  for (int i = 1; i <= 10000; i++) {
    size_t a = (rand() % 2) ? rand() % 2000 + 1000 : rand() % 100 + 50;
    loc = pmm->alloc(a);
    if (i % 2 == 0) pmm->free(loc);
  }
  printf("end4\n");
}

void test5(int tid) {
  void* loc;
  for (int i = 1; i <= 10000; i++) {
    size_t a = rand() % 128 + 5;
    loc = pmm->alloc(a);
    if (i % 5 == 0) pmm->free(loc);
  }
  printf("end5\n");
}



static void os_init() {
  pmm->init();
}
static void os_run() {
  for (const char *s = "Hello World from CPU #*\n"; *s; s++) {
    //putch(*s == '*' ? '0' + cpu_current() : *s);
  }
  //test1(1);
  test2(1);
  //test3(1);
  //test4(1);
  //test5(1);
  while (1) ;
}

MODULE_DEF(os) = {
  .init = os_init,
  .run  = os_run,
};
