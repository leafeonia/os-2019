#include <am.h>
#include <amdev.h>
#include <klib.h>

#define SIDE 16
#define FPS 30
#define WHITE 0xffffffff
#define GBD 0
#define USEDBD 1
#define HANGMAN 2
#define FINBD 3

char answer[16]; 
int W,H,chance,left;

void init_screen();
void redraw();
void init_gameboard();
void init_screen();
void reset_game();
void check_letter(int keycode);
void draw_character(char ch, int x, int y, int color,int mode); 
void update_hangman();
void finish();
