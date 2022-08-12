#pragma once
#define VEC_VERSION     "0.2.1"
#define MAXCACHESIZE    65535
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
#define vec_unpack_(v) \
  (char **)&(v)->data, &(v)->length, &(v)->capacity, sizeof(*(v)->data)
#define vec_t(T) \
  struct { T *data; int length, capacity; }
#define vec_init(v) \
  memset((v), 0, sizeof(*(v)))
#define vec_deinit(v) \
  (free((v)->data),   \
   vec_init(v))
#define vec_push(v, val)              \
  (vec_expand_(vec_unpack_(v)) ? -1 : \
   ((v)->data[(v)->length++] = (val), 0), 0)
#define vec_pop(v) \
  (v)->data[--(v)->length]
#define vec_splice(v, start, count)           \
  (vec_splice_(vec_unpack_(v), start, count), \
   (v)->length -= (count))
#define vec_swapsplice(v, start, count)           \
  (vec_swapsplice_(vec_unpack_(v), start, count), \
   (v)->length -= (count))
#define vec_insert(v, idx, val)            \
  (vec_insert_(vec_unpack_(v), idx) ? -1 : \
   ((v)->data[idx] = (val), 0), (v)->length++, 0)
#define vec_sort(v, fn) \
  qsort((v)->data, (v)->length, sizeof(*(v)->data), fn)
#define vec_swap(v, idx1, idx2) \
  vec_swap_(vec_unpack_(v), idx1, idx2)
#define vec_truncate(v, len) \
  ((v)->length = (len) < (v)->length ? (len) : (v)->length)
#define vec_clear(v) \
  ((v)->length = 0)
#define vec_first(v) \
  (v)->data[0]
#define vec_last(v) \
  (v)->data[(v)->length - 1]
#define vec_reserve(v, n) \
  vec_reserve_(vec_unpack_(v), n)
#define vec_compact(v) \
  vec_compact_(vec_unpack_(v))
#define vec_pusharr(v, arr, count)                                       \
  do {                                                                   \
    int i__, n__ = (count);                                              \
    if (vec_reserve_po2_(vec_unpack_(v), (v)->length + n__) != 0) break; \
    for (i__ = 0; i__ < n__; i__++) {                                    \
      (v)->data[(v)->length++] = (arr)[i__];                             \
    }                                                                    \
  } while (0)
#define vec_extend(v, v2) \
  vec_pusharr((v), (v2)->data, (v2)->length)
#define vec_find(v, val, idx)                       \
  do {                                              \
    for ((idx) = 0; (idx) < (v)->length; (idx)++) { \
      if ((v)->data[(idx)] == (val)) break;         \
    }                                               \
    if ((idx) == (v)->length) (idx) = -1;           \
  } while (0)
#define vec_remove(v, val)                    \
  do {                                        \
    int idx__;                                \
    vec_find(v, val, idx__);                  \
    if (idx__ != -1) vec_splice(v, idx__, 1); \
  } while (0)
#define vec_reverse(v)                             \
  do {                                             \
    int i__ = (v)->length / 2;                     \
    while (i__--) {                                \
      vec_swap((v), i__, (v)->length - (i__ + 1)); \
    }                                              \
  } while (0)
#define vec_foreach(v, var, iter)                                \
  if ((v)->length > 0)                                           \
  for ((iter) = 0;                                               \
       (iter) < (v)->length && (((var) = (v)->data[(iter)]), 1); \
       ++(iter))
#define vec_foreach_rev(v, var, iter)                   \
  if ((v)->length > 0)                                  \
  for ((iter) = (v)->length - 1;                        \
       (iter) >= 0 && (((var) = (v)->data[(iter)]), 1); \
       --(iter))
#define vec_foreach_ptr(v, var, iter)                             \
  if ((v)->length > 0)                                            \
  for ((iter) = 0;                                                \
       (iter) < (v)->length && (((var) = &(v)->data[(iter)]), 1); \
       ++(iter))
#define vec_foreach_ptr_rev(v, var, iter)                \
  if ((v)->length > 0)                                   \
  for ((iter) = (v)->length - 1;                         \
       (iter) >= 0 && (((var) = &(v)->data[(iter)]), 1); \
       --(iter))

typedef vec_t(void *) vec_void_t;
typedef vec_t(char *) vec_str_t;
typedef vec_t(int) vec_int_t;
typedef vec_t(char) vec_char_t;
typedef vec_t(float) vec_float_t;
typedef vec_t(double) vec_double_t;
typedef void (*func)();
typedef struct ui_box_t {
  int  id;
  int  x, y;
  int  w, h;
  int  screen;
  char *cache;
  char *watch;
  char last;
  func draw;
  func onclick;
  func onhover;
  void *data1;
  void *data2;
} ui_box_t;
typedef struct ui_evt_t {
  char *c;
  func f;
} ui_evt_t;
typedef vec_t(ui_box_t *) vec_box_t;
typedef vec_t(ui_evt_t *) vec_evt_t;
typedef struct ui_t {
  struct termios tio;
  struct winsize ws;
  vec_box_t      b;
  vec_evt_t      e;
  int            mouse, screen,
                 scroll, canscroll,
                 id, force;
} ui_t;

int vec_expand_(char **data, int *length, int *capacity, int memsz);
int vec_reserve_(char **data, int *length, int *capacity, int memsz, int n);
int vec_reserve_po2_(char **data, int *length, int *capacity, int memsz, int n);
int vec_compact_(char **data, int *length, int *capacity, int memsz);
int vec_insert_(char **data, int *length, int *capacity, int memsz, int idx);
void vec_splice_(char **data, int *length, int *capacity, int memsz, int start, int count);
void vec_swapsplice_(char **data, int *length, int *capacity, int memsz, int start, int count);
void vec_swap_(char **data, int *length, int *capacity, int memsz, int idx1, int idx2);
char *_strdup_escaped(char *tmp);
int vec_expand_(char **data, int *length, int *capacity, int memsz);
int vec_reserve_(char **data, int *length, int *capacity, int memsz, int n);
int vec_reserve_po2_(char **data, int *length, int *capacity, int memsz, int n);
int vec_compact_(char **data, int *length, int *capacity, int memsz);
int vec_insert_(char **data, int *length, int *capacity, int memsz, int idx);
void vec_splice_(char **data, int *length, int *capacity, int memsz, int start, int count);
void vec_swapsplice_(char **data, int *length, int *capacity, int memsz, int start, int count);
void vec_swap_(char **data, int *length, int *capacity, int memsz, int idx1, int idx2);
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
