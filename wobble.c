/* wobble.c */

#include <stdint.h>
#include <string.h>
#include <math.h>

#include <jd_pretty.h>

#include "wobble.h"
#include "yuv4mpeg2.h"

#define SHIFT 23

typedef struct {
  y4m2_output *out;

  struct {
    double decay;
    double scale;
  } plane[Y4M2_N_PLANE];

  double *acc, *model;
  y4m2_frame *prev, *tmp;

} wobble__work;

static void wobble__free(wobble__work *wrk) {
  if (wrk) {
    y4m2_free(wrk->acc);
    y4m2_free(wrk->model);
    if (wrk->prev) y4m2_release_frame(wrk->prev);
    if (wrk->tmp) y4m2_release_frame(wrk->tmp);
    y4m2_free(wrk);
  }
}

static void wobble__frame(y4m2_frame *frame, const y4m2_parameters *parms, wobble__work *wrk) {
  if (!wrk->acc) wrk->acc = y4m2_alloc(frame->i.size * sizeof(double));
  if (!wrk->model) wrk->model = y4m2_alloc(frame->i.size * sizeof(double));

  if (wrk->prev) {
    double *ap = wrk->acc;
    double *mp = wrk->model;
    uint8_t *fp = frame->buf;
    uint8_t *pp = wrk->prev->buf;

    for (unsigned p = 0; p < Y4M2_N_PLANE; p++) {
      double decay = wrk->plane[p].decay;
      double scale = wrk->plane[p].scale;
      for (unsigned i = 0; i < frame->i.plane[p].size; i++) {
        *ap = *ap * decay + *pp++ - *fp++;
        *mp++ += *ap++ / scale;
      }
      wrk->plane[p].scale = scale * decay + 1;
    }
  }
  else {
    for (unsigned i = 0; i < frame->i.size; i++)
      wrk->model[i] = frame->buf[i];
  }

  if (!wrk->tmp)
    wrk->tmp = y4m2_clone_frame(frame);

  double *mp = wrk->model;
  uint8_t *tp = wrk->tmp->buf;

  for (unsigned p = 0; p < Y4M2_N_PLANE; p++) {
    double min = (p == Y4M2_Y_PLANE) ? 16 : 0;
    double max = (p == Y4M2_Y_PLANE) ? 235 : 255;
    for (unsigned i = 0; i < frame->i.plane[p].size; i++) {
      double sample = *mp++;
      if (sample < min) sample = min;
      if (sample > max) sample = max;
      *tp++ = (uint8_t) sample;
    }
  }

  y4m2_emit_frame(wrk->out, parms, wrk->tmp);

  if (wrk->prev) y4m2_release_frame(wrk->prev);
  wrk->prev = y4m2_retain_frame(frame);
}

static void wobble__callback(y4m2_reason reason,
                             const y4m2_parameters *parms,
                             y4m2_frame *frame,
                             void *ctx) {
  wobble__work *wrk = (wobble__work *) ctx;
  switch (reason) {
  case Y4M2_START:
    y4m2_emit_start(wrk->out, parms);
    break;
  case Y4M2_FRAME:
    wobble__frame(frame, parms, wrk);
    break;
  case Y4M2_END:
    y4m2_emit_end(wrk->out);
    wobble__free(wrk);
    break;
  }
}

static const char *plane_key[] = { "Y", "Cb", "Cr" };

y4m2_output *wobble_hook(y4m2_output *out, jd_var *opt) {
  wobble__work *wrk = y4m2_alloc(sizeof(wobble__work));
  wrk->out = out;
  for (unsigned p = 0; p < Y4M2_N_PLANE; p++) {
    wrk->plane[p].decay = jd_get_real(jd_lv(opt, "$.%s.decay", plane_key[p]));
    wrk->plane[p].scale = 1;
  }
  return y4m2_output_next(wobble__callback, wrk);
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
