/* massive.c */

#include <stdint.h>
#include <string.h>
#include <math.h>

#include <jd_pretty.h>

#include "massive.h"
#include "yuv4mpeg2.h"

#define SHIFT 23

typedef struct {
  y4m2_output *out;

  struct {
    int disabled;
    double mass; /* default mass */
    double drag;
  } plane[Y4M2_N_PLANE];

  double *mass;
  double *position;
  double *velocity;

} massive__work;

static void massive__free(massive__work *wrk) {
  if (wrk) {
    y4m2_free(wrk->mass);
    y4m2_free(wrk->position);
    y4m2_free(wrk->velocity);
    y4m2_free(wrk);
  }
}

static void massive__setup(y4m2_frame *frame,
                           massive__work *wrk) {
  size_t dsize = frame->i.size * sizeof(double);

  double *mp = wrk->mass = y4m2_alloc(dsize);
  double *pp = wrk->position = y4m2_alloc(dsize);
  double *vp = wrk->velocity = y4m2_alloc(dsize);

  uint8_t *fp = frame->buf;

  for (unsigned p = 0; p < Y4M2_N_PLANE; p++) {
    for (unsigned i = 0; i < frame->i.plane[p].size; i++) {
      *mp++ = wrk->plane[p].mass;
      *pp++ = *fp++;
      *vp++ = 0;
    }
  }
}

static void massive__frame(y4m2_frame *frame,
                           const y4m2_parameters *parms,
                           massive__work *wrk) {
  if (!wrk->position) massive__setup(frame, wrk);

  double *mp = wrk->mass;
  double *pp = wrk->position;
  double *vp = wrk->velocity;
  uint8_t *fp = frame->buf;

  for (unsigned p = 0; p < Y4M2_N_PLANE; p++) {
    if (!wrk->plane[p].disabled) {
      double min = (p == Y4M2_Y_PLANE) ? 16 : 0;
      double max = (p == Y4M2_Y_PLANE) ? 235 : 255;
      double drag = wrk->plane[p].drag;
      for (unsigned i = 0; i < frame->i.plane[p].size; i++) {
        double force = (*fp - *pp) - (*vp * fabs(*vp) * drag);
        *vp += force / *mp++;
        double sample = *pp++ += *vp++;
        if (sample < min) sample = min;
        if (sample > max) sample = max;
        *fp++ = (uint8_t) sample;
      }
    }
  }

  y4m2_emit_frame(wrk->out, parms, frame);
}

static void massive__callback(y4m2_reason reason,
                              const y4m2_parameters *parms,
                              y4m2_frame *frame,
                              void *ctx) {
  massive__work *wrk = (massive__work *) ctx;
  switch (reason) {
  case Y4M2_START:
    y4m2_emit_start(wrk->out, parms);
    break;
  case Y4M2_FRAME:
    massive__frame(frame, parms, wrk);
    break;
  case Y4M2_END:
    y4m2_emit_end(wrk->out);
    massive__free(wrk);
    break;
  }
}

static const char *plane_key[] = { "Y", "Cb", "Cr" };

y4m2_output *massive_hook(y4m2_output *out, jd_var *opt) {
  massive__work *wrk = y4m2_alloc(sizeof(massive__work));
  wrk->out = out;
  for (unsigned p = 0; p < Y4M2_N_PLANE; p++) {
    wrk->plane[p].disabled = jd_get_int(jd_lv(opt, "$.%s.disabled", plane_key[p]));
    wrk->plane[p].mass = jd_get_real(jd_lv(opt, "$.%s.mass", plane_key[p]));
    wrk->plane[p].drag = jd_get_real(jd_lv(opt, "$.%s.drag", plane_key[p]));
  }
  return y4m2_output_next(massive__callback, wrk);
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
