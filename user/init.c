#include "ulib.h"
int printf(char *fmt){
  while(*fmt!='\0'){
    kputc(*fmt++);
  }
  return 0;
}
int main() {
  // Example:
  while(1){
    uptime();
  }
  // printf("pid = %d\n", getpid());
  return 0;
}
