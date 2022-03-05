#include <game.h>

// Operating system is a C program!
int main(const char *args) {
  ioe_init();
  puts("mainargs = \"");
  puts(args); // make run mainargs=xxx
  puts("\"\n");

  splash();
int a=8,b=8;
  puts("Press any key to see its key code...\n");
  while (1) {
    char ch=read_key();
    if(ch=='a')a=(a-1)>0?a-1:a-1+16;
    if(ch=='d')a=(a+1)<16?a+1:a+1-16;
    if(ch=='w')b=(b-1)>0?b-1:b-1+16;
    if(ch=='s')b=(b+1)<16?b+1:b+1-16;
    draw_action(a,b);
  }
  return 0;
}
