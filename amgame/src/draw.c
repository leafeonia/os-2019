#include <game.h>
#define W 128
#define H 8
extern char font8x8_basic[128][8];

static uint32_t gameboard[H][W]; //max word length:16

static inline void draw_character(char ch, int x, int y, int color) {
  int i, j;
  char *p = font8x8_basic[(int)ch];
  for (i = 0; i < 8; i ++) 
    for (j = 0; j < 8; j ++) 
      if ((p[i] >> j) & 1)
        if (x + j < W && y + i < H)
          gameboard[y + i][x + j] = color;
}

void redraw(){
	draw_character('A',0,0,0xffffffff);
	draw_character('B',8,0,0xffffffff);
	draw_rect(&gameboard[0][0],100,100,W,H);
	draw_sync();
}
