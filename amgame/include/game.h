#include <am.h>
#include <amdev.h>
#include <klib.h>

#define SIDE 16
#define FPS 30

static inline void puts(const char *s) {
  for (; *s; s++) _putc(*s);
}

void redraw();
