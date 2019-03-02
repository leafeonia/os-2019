#include <am.h>
#include <amdev.h>
#include <klib.h>

#define SIDE 16
#define FPS 30
#define WHITE 0xffffffff

char answer[16]; 

static inline void puts(const char *s) {
  for (; *s; s++) _putc(*s);
}
int W,H,chance;
void redraw();
void init_gameboard();
void init_screen();
void reset_game();
void check_letter(int keycode);
