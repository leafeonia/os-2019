#include <game.h>


void init_screen();
/*
void splash();
int read_key();
uint32_t uptime();
*/
int W,H;

int main() {
  // Operating system is a C program
  _ioe_init();
  init_screen();
  reset_game();
  unsigned long next_frame = 0;
  int keycode;
  printf("%d %d\n",W,H);
  while (1) {
  	while(uptime() < next_frame);
  	next_frame += 1000 / FPS;
    if((keycode = read_key()) != _KEY_NONE && (!(keycode >> 31))){
    	chance--;
    	check_letter(keycode);
    }
    redraw();
  }
  return 0;
}

void reset_game(){
	chance = 7;
	strcpy(answer,"leafeonia");
	init_gameboard();
}

/*
int read_key() {
  _DEV_INPUT_KBD_t event = { .keycode = _KEY_NONE };
  #define KEYNAME(key) \
    [_KEY_##key] = #key,
  static const char *key_names[] = {
    _KEYS(KEYNAME)
  };
  _io_read(_DEV_INPUT, _DEVREG_INPUT_KBD, &event, sizeof(event));
  if (event.keycode != _KEY_NONE && event.keydown) {
    puts("Key pressed: ");
    puts(key_names[event.keycode]);
    puts("\n");
  }
  int ret = event.keycode;
  if (event.keydown) ret |= 0x8000;
  return ret;
}

int w, h;

void init_screen() {
  _DEV_VIDEO_INFO_t info = {0};
  _io_read(_DEV_VIDEO, _DEVREG_VIDEO_INFO, &info, sizeof(info));
  w = info.width;
  h = info.height;
}

void draw_rect(int x, int y, int w, int h, uint32_t color) {
  uint32_t pixels[w * h]; // WARNING: allocated on stack
  _DEV_VIDEO_FBCTL_t event = {
    .x = x, .y = y, .w = w, .h = h, .sync = 1,
    .pixels = pixels,
  };
  for (int i = 0; i < w * h; i++) {
    pixels[i] = color;
  }
  _io_write(_DEV_VIDEO, _DEVREG_VIDEO_FBCTL, &event, sizeof(event));
}

void splash() {
  for (int x = 0; x * SIDE <= w; x ++) {
    for (int y = 0; y * SIDE <= h; y++) {
      if ((x & 1) ^ (y & 1)) {
        draw_rect(x * SIDE, y * SIDE, SIDE, SIDE, 0xffffff); // white
      }
    }
  }
}

uint32_t uptime() {
  _DEV_TIMER_UPTIME_t uptime;
  _io_read(_DEV_TIMER, _DEVREG_TIMER_UPTIME, &uptime, sizeof(uptime));
  return uptime.lo;
}*/
