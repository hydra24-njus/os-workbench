#include <game.h>

// Operating system is a C program!
int main(const char *args) {
  ioe_init();
  puts("mainargs = \"");
  puts(args); // make run mainargs=xxx
  puts("\"\n");

  splash();
  //puts("Press any key to see its key code...\n");
  while (1) {
    char ch=read_key();
    game_action(ch);
  }
  return 0;
}
