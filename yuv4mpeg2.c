/* yuv4mpeg2.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "yuv4mpeg2.h"

static void *y4m2__alloc(size_t size) {
  void *m = malloc(size);
  if (!m) {
    fprintf(stderr, "Out of memory for %lu bytes\n", (unsigned long) size);
    abort();
  }
  memset(m, 0, size);
  return m;
}

static void y4m2__set(char **slot, const char *value) {
  if (*slot) free(*slot);
  if (value) {
    size_t l = strlen(value);
    *slot = y4m2__alloc(l + 1);
    memcpy(*slot, value, l + 1);
  }
  else {
    *slot = NULL;
  }
}

y4m2_parameters *y4m2_new_parms(void) {
  return y4m2__alloc(sizeof(y4m2_parameters));
}

void y4m2_free_parms(y4m2_parameters *parms) {
  int i;
  for (i = 0; i < Y4M2_PARMS; i++)
    free(parms->parm[i]);
  free(parms);
}

y4m2_parameters *y4m2_clone_parms(const y4m2_parameters *orig) {
  y4m2_parameters *parms = y4m2_new_parms();

  for (int i = 0; i < Y4M2_PARMS; i++)
    y4m2__set(&(parms->parm[i]), orig->parm[i]);

  return parms;
}

int y4m2__get_index(const char *name) {
  if (!name)
    return -1;
  if (strlen(name) != 1 || name[0] < Y4M2_FIRST || name[0] > Y4M2_LAST)
    return -1;
  return  name[0] - Y4M2_FIRST;
}

const char *y4m2_get_parm(const y4m2_parameters *parms, const char *name) {
  int idx = y4m2__get_index(name);
  return idx >= 0 ? parms->parm[idx] : NULL;
}

void y4m2_set_parm(y4m2_parameters *parms, const char *name, const char *value) {
  int idx = y4m2__get_index(name);
  if (idx < 0) {
    fprintf(stderr, "Bad parameter name: %s\n", name);
    abort();
  }
  y4m2__set(&(parms->parm[idx]), value);
}

y4m2_frame *y4m2_new_frame(const y4m2_parameters *parms) {
  (void) parms;
  return NULL;
}

void y4m2_free_frame(y4m2_frame *frame) {
  (void) frame;
}

int y4m2_parse(FILE *in, y4m2_callback cb, void *ctx) {
  (void) in;
  (void) cb;
  (void) ctx;
  return 0;
}

int y4m2_emit_start(FILE *out, const y4m2_parameters *parms) {
  (void) out;
  (void) parms;
  return 0;
}

int y4m2_emit_frame(FILE *out, const y4m2_parameters *parms, const y4m2_frame *frame) {
  (void) out;
  (void) parms;
  (void) frame;
  return 0;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
