#include <game.h>

#define KEYNAME(key) \
  [AM_KEY_##key] = #key,
static const char *key_names[] = {
  AM_KEYS(KEYNAME)
};
void print_key() {
  AM_INPUT_KEYBRD_T event = { .keycode = AM_KEY_NONE };
  ioe_read(AM_INPUT_KEYBRD, &event);
  if (event.keycode != AM_KEY_NONE && event.keydown) {
    puts("Key pressed: ");
    puts(key_names[event.keycode]);
    puts("\n");
    if(strcmp(key_names[event.keycode],"ESCAPE")==0)halt(0);
  }
}

char read_key(){
  AM_INPUT_KEYBRD_T event = { .keycode = AM_KEY_NONE };
  ioe_read(AM_INPUT_KEYBRD, &event);
  if (event.keycode != AM_KEY_NONE && event.keydown) {
    puts("Key pressed: ");
    puts(key_names[event.keycode]);
    puts("\n");
    if(strcmp(key_names[event.keycode],"ESCAPE")==0)halt(0);
    else if(strcmp(key_names[event.keycode],"A")==0)return 'a';
    else if(strcmp(key_names[event.keycode],"W")==0)return 'w';
    else if(strcmp(key_names[event.keycode],"D")==0)return 'd';
    else if(strcmp(key_names[event.keycode],"S")==0)return 's';
  }
  return 0;
}
