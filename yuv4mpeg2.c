/* yuv4mpeg2.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "yuv4mpeg2.h"

static const char *tag[] = {
  "YUV4MPEG2",
  "FRAME"
};

void *y4m2_alloc(size_t size) {
  void *m = malloc(size);
  if (!m) {
    fprintf(stderr, "Out of memory for %lu bytes\n", (unsigned long) size);
    abort();
  }
  memset(m, 0, size);
  return m;
}

void y4m2_free(void *m) {
  free(m);
}

static void y4m2__set(char **slot, const char *value) {
  if (*slot) y4m2_free(*slot);
  if (value) {
    size_t l = strlen(value);
    *slot = y4m2_alloc(l + 1);
    memcpy(*slot, value, l + 1);
  }
  else {
    *slot = NULL;
  }
}

y4m2_parameters *y4m2_new_parms(void) {
  return y4m2_alloc(sizeof(y4m2_parameters));
}

void y4m2_free_parms(y4m2_parameters *parms) {
  int i;
  if (parms) {
    for (i = 0; i < Y4M2_PARMS; i++)
      y4m2_free(parms->parm[i]);
    y4m2_free(parms);
  }
}

y4m2_parameters *y4m2_merge_parms(y4m2_parameters *parms, const y4m2_parameters *merge) {
  if (merge)
    for (int i = 0; i < Y4M2_PARMS; i++)
      if (merge->parm[i])
        y4m2__set(&(parms->parm[i]), merge->parm[i]);
  return parms;
}

y4m2_parameters *y4m2_clone_parms(const y4m2_parameters *orig) {
  return y4m2_merge_parms(y4m2_new_parms(), orig);
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
    exit(1);
  }
  y4m2__set(&(parms->parm[idx]), value);
}

static unsigned parse_num(const char *s) {
  if (s) {
    char *ep;
    unsigned n = (unsigned) strtoul(s, &ep, 10);
    if (ep > s && *ep == '\0') return n;
  }
  fprintf(stderr, "Bad number");
  exit(1);
}

static void set_planes(y4m2_frame_info *info,
                       unsigned xsY, unsigned ysY,
                       unsigned xsCb, unsigned ysCb,
                       unsigned xsCr, unsigned ysCr) {
  size_t pix_size = info->width * info->height;

  info->plane[Y4M2_Y_PLANE].xs = xsY;
  info->plane[Y4M2_Y_PLANE].ys = ysY;
  info->plane[Y4M2_Y_PLANE].size = pix_size / (xsY * ysY);
  info->plane[Y4M2_Cb_PLANE].xs = xsCb;
  info->plane[Y4M2_Cb_PLANE].ys = ysCb;
  info->plane[Y4M2_Cb_PLANE].size = pix_size / (xsCb * ysCb);
  info->plane[Y4M2_Cr_PLANE].xs = xsCr;
  info->plane[Y4M2_Cr_PLANE].ys = ysCr;
  info->plane[Y4M2_Cr_PLANE].size = pix_size / (xsCr * ysCr);

  info->size = info->plane[Y4M2_Y_PLANE].size +
               info->plane[Y4M2_Cb_PLANE].size +
               info->plane[Y4M2_Cr_PLANE].size;
}

void y4m2_parse_frame_info(y4m2_frame_info *info, const y4m2_parameters *parms) {
  info->width = parse_num(y4m2_get_parm(parms, "W"));
  info->height = parse_num(y4m2_get_parm(parms, "H"));

  const char *cs = y4m2_get_parm(parms, "C");
  if (!cs) cs = "420";
  if (!strcmp("420", cs) ||
      !strcmp("420jpeg", cs) ||
      !strcmp("420mpeg2", cs) ||
      !strcmp("420paldv", cs)) {
    set_planes(info, 1, 1, 2, 2, 2, 2);
  }
  else if (!strcmp("422", cs)) {
    set_planes(info, 1, 1, 2, 1, 2, 1);
  }
  else if (!strcmp("444", cs)) {
    set_planes(info, 1, 1, 1, 1, 1, 1);
  }
  else {
    fprintf(stderr, "Unknown colourspace %s\n", cs);
    exit(1);
  }
}

y4m2_frame *y4m2_new_frame(const y4m2_parameters *parms) {
  y4m2_frame *frame;
  y4m2_frame_info info;
  y4m2_parse_frame_info(&info, parms);

  frame = y4m2_alloc(sizeof(y4m2_frame));
  uint8_t *buf = frame->buf = y4m2_alloc(info.size);

  for (int i = 0; i < Y4M2_N_PLANE; i++) {
    frame->plane[i] = buf;
    buf += info.plane[i].size;
  }

  frame->i = info;
  return y4m2_retain_frame(frame);
}

y4m2_frame *y4m2_clone_frame(const y4m2_frame *frame) {
  y4m2_frame *nf = y4m2_alloc(sizeof(y4m2_frame));
  *nf = *frame;
  nf->buf = y4m2_alloc(frame->i.size);
  memcpy(nf->buf, frame->buf, frame->i.size);
  nf->refcnt = 0;
  return y4m2_retain_frame(nf);
}

void y4m2_free_frame(y4m2_frame *frame) {
  if (frame) {
    y4m2_free(frame->buf);
    y4m2_free(frame);
  }
}

y4m2_frame *y4m2_retain_frame(y4m2_frame *frame) {
  frame->refcnt ++;
  return frame;
}

void y4m2_release_frame(y4m2_frame *frame) {
  if (--frame->refcnt == 0)
    y4m2_free_frame(frame);
}

static char *is_word(char *buf, const char *match) {
  size_t l = strlen(match);
  if (strlen(buf) >= l && memcmp(buf, match, l) == 0 && buf[l] <= ' ')
    return buf + l;
  return NULL;
}

void y4m2__parse_parms(y4m2_parameters *parms, char *buf) {
  char name[2];
  for (;;) {
    while (*buf == ' ') buf++;
    if (*buf < ' ') break;
    name[0] = *buf++;
    name[1] = '\0';
    char *vp = buf;
    while (*buf > ' ') buf++;
    char t = *buf;
    *buf = '\0';
    /*    fprintf(stderr, "%s=%s\n", name, vp);*/
    y4m2_set_parm(parms, name, vp);
    *buf = t;
  }
}

void y4m2__format_parms(FILE *out, const y4m2_parameters *parms) {
  for (int i = 0; i < Y4M2_PARMS; i++)
    if (parms->parm[i])
      fprintf(out, " %c%s", Y4M2_FIRST + i, parms->parm[i]);
}

int y4m2_parse(FILE *in, y4m2_output *out) {
  size_t buf_size = 0;
  char *buf = NULL;
  y4m2_parameters *global = NULL;

  for (;;) {
    int c = getc(in);
    unsigned pos = 0;
    for (;;) {
      if (pos == buf_size) {
        buf_size *= 2;
        if (buf_size < 1024) buf_size = 1024;
        char *nb = realloc(buf, buf_size);
        if (NULL == nb) abort();
        buf = nb;
      }
      if (c == EOF) goto done;
      if (c < ' ') {
        buf[pos++] = '\0';
        break;
      }
      buf[pos++] = c;
      c = getc(in);
    }
    if (c == EOF) break;

    char *tail;
    if (tail = is_word(buf, tag[Y4M2_START]), tail) {
      if (global) y4m2_free_parms(global);
      global = y4m2_new_parms();
      y4m2__parse_parms(global, tail);
      y4m2_emit_start(out, global);
    }
    else if (tail = is_word(buf, tag[Y4M2_FRAME]), tail) {
      y4m2_parameters *parms = y4m2_new_parms();
      y4m2__parse_parms(parms, tail);

      y4m2_parameters *merged = y4m2_clone_parms(global);
      y4m2_merge_parms(merged, parms);

      y4m2_frame *frame = y4m2_new_frame(merged);
      size_t got = fread(frame->buf, 1, frame->i.size, in);
      if (got != frame->i.size) {
        fprintf(stderr, "Short read");
        exit(1);
      }
      y4m2_emit_frame(out, parms, frame);
      y4m2_release_frame(frame);
      y4m2_free_parms(parms);
      y4m2_free_parms(merged);
    }
    else {
      fprintf(stderr, "Bad stream");
      exit(1);
    }
  }

done:

  y4m2_emit_end(out);

  return 0;
}

int y4m2_emit_start(y4m2_output *out, const y4m2_parameters *parms) {
  switch (out->type) {
  case Y4M2_OUTPUT_FILE:
    fputs(tag[Y4M2_START], out->o.f);
    y4m2__format_parms(out->o.f, parms);
    fputc(0x0A, out->o.f);
    break;
  case Y4M2_OUTPUT_NEXT:
    out->o.n.cb(Y4M2_START, parms, NULL, out->o.n.ctx);
    break;
  }
  return 0;
}

int y4m2_emit_frame(y4m2_output *out, const y4m2_parameters *parms, y4m2_frame *frame) {
  switch (out->type) {
  case Y4M2_OUTPUT_FILE:
    fputs(tag[Y4M2_FRAME], out->o.f);
    y4m2__format_parms(out->o.f, parms);
    fputc(0x0A, out->o.f);
    fwrite(frame->buf, 1, frame->i.size, out->o.f);
    break;
  case Y4M2_OUTPUT_NEXT:
    out->o.n.cb(Y4M2_FRAME, parms, frame, out->o.n.ctx);
    break;
  }
  return 0;
}

int y4m2_emit_end(y4m2_output *out) {
  switch (out->type) {
  case Y4M2_OUTPUT_FILE:
    break;
  case Y4M2_OUTPUT_NEXT:
    out->o.n.cb(Y4M2_END, NULL, NULL, out->o.n.ctx);
    break;
  }
  return 0;
}

y4m2_output *y4m2_output_file(FILE *out) {
  y4m2_output *o = y4m2_alloc(sizeof(y4m2_output));
  o->type = Y4M2_OUTPUT_FILE;
  o->o.f = out;
  return o;
}

y4m2_output *y4m2_output_next(y4m2_callback cb, void *ctx) {
  y4m2_output *o = y4m2_alloc(sizeof(y4m2_output));
  o->type = Y4M2_OUTPUT_NEXT;
  o->o.n.cb = cb;
  o->o.n.ctx = ctx;
  return o;
}

void y4m2_free_output(y4m2_output *out) {
  y4m2_free(out);
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
