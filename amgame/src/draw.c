#include <game.h>
#define gameboard_W 128
#define gameboard_H 8

extern char font8x8_basic[128][8];

static uint32_t gameboard[gameboard_H][gameboard_W]; //max word length:16

static inline void draw_character(char ch, int x, int y, int color) {
  int i, j;
  char *p = font8x8_basic[(int)ch];
  for (i = 0; i < 8; i ++) 
    for (j = 0; j < 8; j ++) 
      if ((p[i] >> j) & 1)
        if (x + j < gameboard_W && y + i < gameboard_W)
          gameboard[y + i][x + j] = color;
}

void redraw(){
	draw_character('A',0,0,0xffffffff);
	draw_character('B',8,0,0xffff);
	draw_rect(&gameboard[0][0],100,100,gameboard_W,gameboard_H);
	draw_sync();
}

void init_gameboard(){
	int i;
	for(i = 0;i < 8;i++){
		draw_character('_',8*i,0,0xffffffff);
	}
	draw_rect(&gameboard[0][0],200,200,gameboard_W,gameboard_H);
	draw_sync();
}

void init_screen(){
	W = screen_width();
	H = screen_height();
}
