#include <game.h>

#define SIDE 16
static int w, h;
static int a,b;
void game_init();
static void init() {
  AM_GPU_CONFIG_T info = {0};
  ioe_read(AM_GPU_CONFIG, &info);
  w = info.width;
  h = info.height;
  game_init();
}

static void draw_tile(int x, int y, int w, int h, uint32_t color) {
  uint32_t pixels[w * h]; // WARNING: large stack-allocated memory
  AM_GPU_FBDRAW_T event = {
    .x = x, .y = y, .w = w, .h = h, .sync = 1,
    .pixels = pixels,
  };
  for (int i = 0; i < w * h; i++) {
    pixels[i] = color;
  }
  ioe_write(AM_GPU_FBDRAW, &event);
}

void splash() {
  init();  printf("%d %d\n",w,h);
  for (int x = 0; x * SIDE <= w; x ++) {
    for (int y = 0; y * SIDE <= h; y++) {
      if ((x & 1) ^ (y & 1)) {
        draw_tile(x * SIDE, y * SIDE, SIDE, SIDE, 0xffffff); // white
      }
    }
  }
}
void draw_action(int i,int j){
    for (int x = 0; x * SIDE <= w; x ++) {
    for (int y = 0; y * SIDE <= h; y++) {
      if (x!=i||y!=j)
        draw_tile(x * SIDE, y * SIDE, SIDE, SIDE, 0x000000); // white
      else draw_tile(x * SIDE, y * SIDE, SIDE, SIDE, 0xffffff); 
    }
  }
}
void game_init(){
  a=w/(2*SIDE),b=h/(2*SIDE);
  printf("a=%d b=%d\n",a,b);
}
void game_action(char ch){
  if(ch=='a')a=(a-1)>=0?a-1:a-1+w/SIDE;
  if(ch=='d')a=(a+1)<w/SIDE?a+1:a+1-w/SIDE;
  if(ch=='w')b=(b-1)>=0?b-1:b-1+h/SIDE;
  if(ch=='s')b=(b+1)<h/SIDE?b+1:b+1-h/SIDE;
  draw_action(a,b);
}
