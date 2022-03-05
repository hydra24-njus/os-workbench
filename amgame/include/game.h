#include <am.h>
#include <amdev.h>
#include <klib.h>
#include <klib-macros.h>
void init();
void splash();
void print_key();
char read_key();
void draw_action(int x,int y);
void game_action(char ch);
static inline void puts(const char *s) {
  for (; *s; s++) putch(*s);
}
