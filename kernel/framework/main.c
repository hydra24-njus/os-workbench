#include <common.h>
#include <klib.h>

void test(){
  task_t *p=NULL;
  if(p==NULL)printf("1\n");
}
int main() {
  ioe_init();
  cte_init(os->trap);
  os->init();
  mpe_init(test);
  return 1;
}
