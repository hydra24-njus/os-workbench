#include <common.h>
#include <klib.h>

int main() {
  ioe_init();
  cte_init(os->trap);
  os->init();
  task_t *p=NULL;
  if(p)printf("1\n");
  mpe_init(os->run);
  return 1;
}
