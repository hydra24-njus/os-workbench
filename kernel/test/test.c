#include <common.h>
#include <thread.h>

static void test0(int tid) {
  void* loc=pmm->alloc(16<<10);
  void* loc2=pmm->alloc(16<<10);
  pmm->free(loc2);
  pmm->free(loc);
}

static void test1(int tid) {
  void* loc;
  for (int i = 1; i <= 2000; i++) {
    loc = pmm->alloc(5000 + (i * 1000) % 5000);
    // if (i % 5 == 0) pmm->free(loc);
  }
}

static void test2(int tid) {
  void* loc;
  for (int i = 1; i <= 10000; i++) {
    size_t a = (rand() % 2) ? rand() % 2000 + 1000 : rand() % 100 + 50;
    loc = pmm->alloc(a);
    //printf("loc=%p\n",loc);
    //if(loc==NULL)assert(0);
    if (i % 5 == 0) pmm->free(loc);
  }
}

static void test3(int tid) {
  void* loc;
  for (int i = 1; i <= 10000; i++) {
    loc = pmm->alloc(5000 + (i * 1000) % 5000);
    pmm->free(loc);
  }
}

static void test4(int tid) {
  void* loc;
  for (int i = 1; i <= 10000; i++) {
    size_t a = (rand() % 2) ? rand() % 2000 + 1000 : rand() % 100 + 50;
    loc = pmm->alloc(a);
    if (i % 2 == 0) pmm->free(loc);
  }
}

static void test5(int tid) {
  void* loc;
  for (int i = 1; i <= 10000; i++) {
    size_t a = rand() % 128 + 5;
    loc = pmm->alloc(a);
    if (i % 5 == 0) pmm->free(loc);
  }
}
int main() {
  clock_t start_time, end_time;
  // start_time = clock();
  pmm->init();
  create(test5);
  join();
  end_time = clock();  // 结束时间
  /* 计算得出程序运行时间, 并将其输出到屏幕 */
  printf("%lf\n", (double)(end_time - start_time) / CLOCKS_PER_SEC);
}