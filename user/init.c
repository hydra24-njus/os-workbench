#include "ulib.h"

int main() {
  // Example:
  kputc('h');
  kputc('e');
  kputc('l');
  kputc('l');
  kputc('o');
  kputc('\n');
  while(1){
    kputc('h');
    kputc('e');
    kputc('l');
    kputc('l');
    kputc('o');
    kputc('\n');
    sleep(1);
  }
  // printf("pid = %d\n", getpid());
  return 0;
}
