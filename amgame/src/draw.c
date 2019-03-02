#include <game.h>
#define gameboard_W 128
#define gameboard_H 8
#define usedboard_W 96
#define usedboard_H 8
#define hangman_W 40
#define hangman_H 40
extern char font8x8_basic[128][8];
extern char charimg[7][5][5];

uint32_t gameboard[gameboard_H][gameboard_W]; //max word length:16
uint32_t usedboard[usedboard_H][usedboard_W];
uint32_t hangman[hangman_H][hangman_W];
static int gbd_x,gbd_y; //gbd = gameboard
static int usedbd_x,usedbd_y; //used board
static int hangman_x,hangman_y;

void draw_character(char ch, int x, int y, int color ,int mode) {
  int i, j;
  char *p = font8x8_basic[(int)ch];
  for (i = 0; i < 8; i ++) 
    for (j = 0; j < 8; j ++) 
      if ((p[i] >> j) & 1){
      	if(mode == GBD){
        	if (x + j < gameboard_W && y + i < gameboard_W)
          		gameboard[y + i][x + j] = color;
        }
        else if(mode == USEDBD){
        	if (x + j < usedboard_W && y + i < usedboard_W)
          		usedboard[y + i][x + j] = color;
        }
        else if(mode == HANGMAN){
        	if (x + j < hangman_W && y + i < hangman_W)
          		hangman[y + i][x + j] = color;
        }
      } 
}

void redraw(){
	draw_rect(&gameboard[0][0],gbd_x,gbd_y,gameboard_W,gameboard_H);
	draw_rect(&usedboard[0][0],usedbd_x,usedbd_y,usedboard_W,usedboard_H);
	draw_rect(&hangman[0][0],hangman_x,hangman_y,hangman_W,hangman_H);
	draw_sync();
}

void init_gameboard(){
	int i;
	//clear the screen first
	memset(gameboard,0,sizeof(gameboard));
	memset(usedboard,0,sizeof(usedboard));
	redraw();	
	
	update_hangman();
	for(i = 0;i < strlen(answer);i++){
		draw_character('_',8*i,0,WHITE,GBD);
	}
	char temp[] = {'u','s','e','d',':'};
	for(i = 0;i < 5;i++)
		draw_character(temp[i],8*i,0,WHITE,USEDBD);

	redraw();
}

void init_screen(){
	W = screen_width();
	H = screen_height();
	gbd_x = (W - gameboard_W) / 2;
	gbd_y = (H - gameboard_H) / 2;
	usedbd_x = gbd_x;
	usedbd_y = gbd_y + 30;
	hangman_x = gbd_x + 200;
	hangman_y = gbd_y;
}

void update_hangman(){
	int i,j;
	for(i = 0;i < 5;i++){
		for(j = 0;j < 5;j++){
			draw_character(charimg[chance][i][j],8*i,8*j,WHITE,HANGMAN);
		}
	}
}


