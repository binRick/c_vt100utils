#pragma once
#define VEC_VERSION     "0.2.1"
#define MAXCACHESIZE    65535
#include "tuibox-vec.h"
#include <termios.h>
////////////////////////////////////////////////////
enum binding_mode_t {
  BINDING_MODE_MOUSE_SCROLL_UP,
  BINDING_MODE_MOUSE_SCROLL_DOWN,
  BINDING_MODE_UNHANDLED_INPUT,
  BINDING_MODES_QTY,
};
struct binding_type_t {
  void (*handler)(void *);
};

#define CURSOR_Y(b)              (b->y + (n + 1) + (u->canscroll ? u->scroll : 0))
#define box_contains(x, y, b)    (x >= b->x && x <= b->x + b->w && y >= b->y && y <= b->y + b->h)
#define ui_screen(s, u)          u->screen = s; u->force = 1
#define ui_center_x(w, u)        (((u)->ws.ws_col - w) / 2)
#define ui_center_y(h, u)        (((u)->ws.ws_row - h) / 2)
#define UI_CENTER_X    -1
#define UI_CENTER_Y    -1
#define ui_loop(u)     \
  char buf[64]; int n; \
  while ((n = read(STDIN_FILENO, buf, sizeof(buf))) > 0)
#define ui_update(u)     _ui_update(buf, n, u)
#define ui_get(id, u)    ((u)->b.data[id])
#define COORDINATE_DECODE() \
  tok = strtok(NULL, ";");  \
  x   = atoi(tok);          \
  tok = strtok(NULL, ";");  \
  y   = strtol(tok, NULL, 10) - (u->canscroll ? u->scroll : 0)
#define LOOP_AND_EXECUTE(f)           \
  do {                                \
    vec_foreach(&(u->b), tmp, ind){   \
      if (tmp->screen == u->screen && \
          f != NULL &&                \
          box_contains(x, y, tmp)) {  \
        f(tmp, x, y, u->mouse);       \
      }                               \
    }                                 \
  } while (0)

//////////////////////////////////////////////////////////////////////////////////////////////
void ui_new(int s, ui_t *u);
void ui_free(ui_t *u);
int ui_add(int x, int y, int w, int h, int screen, char *watch, char initial, func draw, func onclick, func onhover, void *data1, void *data2, ui_t *u);
void ui_key(char *c, func f, ui_t *u);
void ui_clear(ui_t *u);
void ui_draw_one(ui_box_t *tmp, int flush, ui_t *u);
void ui_draw(ui_t *u);
void ui_redraw(ui_t *u);
void _ui_update(char *c, int n, ui_t *u);
void _ui_text(ui_box_t *b, char *out);
int ui_text(int x, int y, char *str, int screen, func click, func hover, ui_t *u);

//////////////////////////////////////////////////////////////////////////////////////////////
