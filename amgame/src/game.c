#include <game.h>

// Operating system is a C program!
static int w,h;
int main(const char *args) {
  ioe_init();
  puts("mainargs = \"");
  puts(args); // make run mainargs=xxx
  puts("\"\n");

  splash();
int a=w/2,b=h/2;
  puts("Press any key to see its key code...\n");
  while (1) {
    char ch=read_key();
    if(ch=='a')a=(a-1)>0?a-1:a-1+w;
    if(ch=='d')a=(a+1)<16?a+1:a+1-w;
    if(ch=='w')b=(b-1)>0?b-1:b-1+h;
    if(ch=='s')b=(b+1)<16?b+1:b+1-h;
    draw_action(a,b);
  }
  return 0;
}
