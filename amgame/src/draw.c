#include <game.h>
#define gameboard_W 128
#define gameboard_H 8

extern char font8x8_basic[128][8];

uint32_t gameboard[gameboard_H][gameboard_W]; //max word length:16
static int gbd_x,gbd_y; //gbd = gameboard

void draw_character(char ch, int x, int y, int color) {
  int i, j;
  char *p = font8x8_basic[(int)ch];
  for (i = 0; i < 8; i ++) 
    for (j = 0; j < 8; j ++) 
      if ((p[i] >> j) & 1)
        if (x + j < gameboard_W && y + i < gameboard_W)
          gameboard[y + i][x + j] = color;
}

void redraw(){
	//draw_character('A',0,0,0xffffffff);
	//draw_character('B',8,0,0xffff);
	draw_rect(&gameboard[0][0],gbd_x,gbd_y,gameboard_W,gameboard_H);
	draw_sync();
}

void init_gameboard(){
	int i;
	for(i = 0;i < strlen(answer);i++){
		draw_character('_',8*i,0,0xffffffff);
	}
	draw_rect(&gameboard[0][0],gbd_x,gbd_y,gameboard_W,gameboard_H);
	draw_sync();
}

void init_screen(){
	W = screen_width();
	H = screen_height();
	gbd_x = (W - gameboard_W) / 2;
	gbd_y = (H - gameboard_H) / 2;
}


