#pragma once
#include <stdint.h>
///////////////////////////////////////////////////////
#define MAX(a, b)    (a > b ? a : b)
///////////////////////////////////////////////////////
struct vt100_color_t {
  enum {
    palette_8,
    palette_8_bright,
    palette_256,
    truecolor
  }        type;
  uint32_t value;
};

struct vt100_node_t {
  char                 *str;
  int                  len;
  struct vt100_color_t fg;
  struct vt100_color_t bg;
  uint8_t              mode;
  struct vt100_node_t  *next;
};
///////////////////////////////////////////////////////

struct vt100_node_t *vt100_decode(char *str);
char *vt100_sgr(struct vt100_node_t *node, struct vt100_node_t *prev);
char *vt100_encode(struct vt100_node_t *node);
void vt100_free(struct vt100_node_t *head);
///////////////////////////////////////////////////////
