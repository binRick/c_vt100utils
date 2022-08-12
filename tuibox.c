#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
////////////////////////////////////////////////
#include "tuibox.h"
////////////////////////////////////////////////
#include "tuibox-vec.h"
////////////////////////////////////////////////
struct binding_type_t *binding_types[] = {
  [BINDING_MODE_MOUSE_SCROLL_UP] = &(struct binding_type_t)  {
    .handler = NULL,
  },
  [BINDING_MODE_MOUSE_SCROLL_DOWN] = &(struct binding_type_t){
    .handler = NULL,
  },
  [BINDING_MODE_UNHANDLED_INPUT] = &(struct binding_type_t)  {
    .handler = NULL,
  },
  [BINDING_MODES_QTY] = NULL,
};


char *_strdup_escaped(char *tmp) {
  char *ret = (char *)malloc(strlen(tmp) * 4 + 1);
  char *dst = ret;

  for ( ; *tmp; tmp++) {
    if (*tmp >= ' ' && *tmp <= 126 && *tmp != '\\') {
      *dst = *tmp;
      ++dst;
    } else {
      dst += sprintf(dst, "\\x%02hhx", (unsigned char)*tmp);
    }
  }
  *dst = 0;
  return(ret);
}


void ui_new(int s, ui_t *u){
  struct termios raw;

  ioctl(STDOUT_FILENO, TIOCGWINSZ, &(u->ws));

  tcgetattr(STDIN_FILENO, &(u->tio));
  raw          = u->tio;
  raw.c_lflag &= ~(ECHO | ICANON);
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);

  vec_init(&(u->b));
  vec_init(&(u->e));
  printf("\x1b[?1049h\x1b[0m\x1b[2J\x1b[?1003h\x1b[?1015h\x1b[?1006h\x1b[?25l");
  printf("\x1b[?1004h");
  printf("\x1b[?1002h");
  printf("\x1b[?1000h");
  fflush(stdout);


  u->mouse = 1;

  u->screen    = s;
  u->scroll    = 1;
  u->canscroll = 0;

  u->id = 0;

  u->force = 0;
}


void ui_free(ui_t *u){
  ui_box_t *val;
  ui_evt_t *evt;
  int      i;
  char     *term;

  printf("\x1b[0m\x1b[2J\x1b[?1049l\x1b[?1003l\x1b[?1015l\x1b[?1006l\x1b[?25h");
  printf("\x1b[?1004l");
  fflush(stdout);
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &(u->tio));


  vec_foreach(&(u->b), val, i){
    free(val->cache);
    free(val);
  }
  vec_deinit(&(u->b));

  vec_foreach(&(u->e), evt, i){
    free(evt);
  }
  vec_deinit(&(u->e));

  term = getenv("TERM");
  if (strncmp(term, "screen", 6) == 0
      || strncmp(term, "tmux", 4) == 0) {
    printf("Note: Terminal multiplexer detected.\n  For best performance (i.e. reduced flickering), running natively inside\n  a GPU-accelerated terminal such as alacritty or kitty is recommended.\n");
  }
}


int ui_add(int x, int y, int w, int h, int screen,
           char *watch, char initial,
           func draw, func onclick, func onhover,
           void *data1, void *data2,
           ui_t *u) {
  char     *buf = malloc(MAXCACHESIZE);

  ui_box_t *b = malloc(sizeof(ui_box_t));

  b->id = u->id++;

  b->x = (x == UI_CENTER_X ? ui_center_x(w, u) : x);
  b->y = (y == UI_CENTER_Y ? ui_center_y(h, u) : y);
  b->w = w;
  b->h = h;

  b->screen = u->screen;

  b->watch = watch;
  b->last  = initial;

  b->draw    = draw;
  b->onclick = onclick;
  b->onhover = onhover;

  b->data1 = data1;
  b->data2 = data2;

  draw(b, buf);
  b->cache = realloc(buf, strlen(buf) * 2);

  vec_push(&(u->b), b);

  return(b->id);
}


/*
 * Adds a new key event listener
 *   to the UI.
 */
void ui_key(char *c, func f, ui_t *u){
  ui_evt_t *e = malloc(sizeof(ui_evt_t));

  e->c = c;
  e->f = f;

  vec_push(&(u->e), e);
}


void ui_clear(ui_t *u){
  int tmp = u->screen;

  ui_free(u);
  ui_new(tmp, u);
}


/*
 * Draws a single box to the
 *   screen.
 */
void ui_draw_one(ui_box_t *tmp, int flush, ui_t *u){
  char *buf, *tok;
  int  n = -1;

  if (tmp->screen != u->screen) {
    return;
  }

  buf = calloc(1, strlen(tmp->cache) * 2);
  if (u->force
      || tmp->watch == NULL
      || *(tmp->watch) != tmp->last
      ) {
    tmp->draw(tmp, buf);
    if (tmp->watch != NULL) {
      tmp->last = *(tmp->watch);
    }
    strcpy(tmp->cache, buf);
  } else {
    /* buf is allocated proportionally to tmp->cache, so strcpy is safe */
    strcpy(buf, tmp->cache);
  }
  tok = strtok(buf, "\n");
  while (tok != NULL) {
    if (tmp->x > 0
        && tmp->x < u->ws.ws_col
        && CURSOR_Y(tmp) > 0
        && CURSOR_Y(tmp) < u->ws.ws_row) {
      printf("\x1b[%i;%iH%s", CURSOR_Y(tmp), tmp->x, tok);
      n++;
    }
    tok = strtok(NULL, "\n");
  }
  free(buf);

  if (flush) {
    fflush(stdout);
  }
}


/*
 * Draws all boxes to the screen.
 */
void ui_draw(ui_t *u){
  ui_box_t *tmp;
  int      i;

  printf("\x1b[0m\x1b[2J");

  vec_foreach(&(u->b), tmp, i){
    ui_draw_one(tmp, 0, u);
  }
  fflush(stdout);
  u->force = 0;
}


void ui_redraw(ui_t *u){
  u->force = 1;
  ui_draw(u);
}


void _ui_update(char *c, int n, ui_t *u){
  ui_box_t *tmp;
  ui_evt_t *evt;
  int      ind, x, y;
  char     cpy[n], *tok;
  bool     is_handled = false;

  if (n >= 4
      && c[0] == '\x1b'
      && c[1] == '['
      && c[2] == '<') {
    strncpy(cpy, c, n);
    tok = strtok(cpy + 3, ";");
    if (strcmp(tok, "64") == 0) {
      if (binding_types[BINDING_MODE_MOUSE_SCROLL_UP] != NULL && binding_types[BINDING_MODE_MOUSE_SCROLL_UP]->handler != NULL) {
        binding_types[BINDING_MODE_MOUSE_SCROLL_UP]->handler((void *)NULL);
        ui_draw(u);
        is_handled = true;
      }
    }else if (strcmp(tok, "65") == 0) {
      if (binding_types[BINDING_MODE_MOUSE_SCROLL_DOWN] != NULL && binding_types[BINDING_MODE_MOUSE_SCROLL_DOWN]->handler != NULL) {
        binding_types[BINDING_MODE_MOUSE_SCROLL_DOWN]->handler((void *)NULL);
        ui_draw(u);
        is_handled = true;
      }
    }else{
      switch (tok[0]) {
      case '0':
        u->mouse = (strchr(c, 'm') == NULL);
        if (u->mouse) {
          COORDINATE_DECODE();
          LOOP_AND_EXECUTE(tmp->onclick);
          is_handled = true;
        }
        break;
      case '3':
        u->mouse = (strcmp(tok, "32") == 0);
        COORDINATE_DECODE();
        LOOP_AND_EXECUTE(tmp->onhover);
        is_handled = true;
        break;
      case '6':
        if (u->canscroll) {
          u->scroll += (4 * (tok[1] == '4')) - 2;
          printf("\x1b[0m\x1b[2J");
          ui_draw(u);
          is_handled = true;
        }
        break;
      }
    }
  }

  if (false == is_handled) {
    vec_foreach(&(u->e), evt, ind){
      if (strncmp(c, evt->c, strlen(evt->c)) == 0) {
        evt->f();
        is_handled = true;
      }else{
        char _cpy[n], *_tok, *__tok, *___tok;
        strncpy(_cpy, _strdup_escaped(c), strlen(_strdup_escaped(c)));
        _tok   = strtok(_cpy + 0, "\\x");
        __tok  = strtok(_tok + 0, "1b");
        ___tok = strtok(_tok + 0, "09");
        if (strncmp(__tok, evt->c, strlen(evt->c)) == 0) {
          evt->f();
          is_handled = true;
        }else if (strcmp(c, evt->c) == 0) {
          evt->f();
          is_handled = true;
        }else if (strcmp(_strdup_escaped(c), _strdup_escaped(evt->c)) == 0) {
          evt->f();
          is_handled = true;
        }else{
        }
      }
    }
  }
  if (false == is_handled) {
    if (binding_types[BINDING_MODE_UNHANDLED_INPUT] != NULL && binding_types[BINDING_MODE_UNHANDLED_INPUT]->handler != NULL) {
      binding_types[BINDING_MODE_UNHANDLED_INPUT]->handler((void *)c);
      is_handled = true;
      ui_draw(u);
    }

    if (false == is_handled) {
      fprintf(stderr,
              "unhandled "
              "\n\tc: '%s'||'%s' (%lu chars)"
              "\n",
              _strdup_escaped(c), c, strlen(c)
              );
    }
  }
} // _ui_update


void _ui_text(ui_box_t *b, char *out){
  sprintf(out, "%s", (char *)b->data1);
}


int ui_text(int x, int y, char *str,
            int screen,
            func click, func hover,
            ui_t *u) {
  return(ui_add(
           x, y,
           strlen(str), 1,
           screen,
           NULL, 0,
           _ui_text,
           click,
           hover,
           str,
           NULL,
           u
           ));
}

