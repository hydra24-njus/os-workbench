#include <common.h>
#include <klib.h>

int main() {
  task_t *p=NULL;
  if(p)printf("1\n");
  ioe_init();
  cte_init(os->trap);
  os->init();
  mpe_init(os->run);
  return 1;
}
