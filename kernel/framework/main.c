#include <common.h>
#include <klib.h>

int main() {
  ioe_init();
  cte_init(os->trap);
    task_t *p=NULL;
  if(p)printf("1\n");
  os->init();
  mpe_init(os->run);
  return 1;
}
