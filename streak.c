/* streak.c */

#include <stdint.h>
#include <string.h>
#include <math.h>

#include <jd_pretty.h>

#include "streak.h"
#include "yuv4mpeg2.h"

#define SHIFT 23

typedef struct {
  y4m2_output *out;
  double *acc;
  double decay;
  double scale;
} streak__work;

static void streak__free(streak__work *wrk) {
  if (wrk) {
    y4m2_free(wrk->acc);
    y4m2_free(wrk);
  }
}

static void streak__frame(y4m2_frame *frame, const y4m2_parameters *parms, streak__work *wrk) {
  if (!wrk->acc) wrk->acc = y4m2_alloc(frame->i.size * sizeof(double));

  for (unsigned i = 0; i < frame->i.size; i++) {
    wrk->acc[i] = wrk->acc[i] * wrk->decay + frame->buf[i];
    frame->buf[i] = (uint8_t)(wrk->acc[i] / wrk->scale);
  }

  wrk->scale = wrk->scale * wrk->decay + 1;

  y4m2_emit_frame(wrk->out, parms, frame);
}

static void streak__callback(y4m2_reason reason,
                             const y4m2_parameters *parms,
                             y4m2_frame *frame,
                             void *ctx) {
  streak__work *wrk = (streak__work *) ctx;
  switch (reason) {
  case Y4M2_START:
    y4m2_emit_start(wrk->out, parms);
    break;
  case Y4M2_FRAME:
    streak__frame(frame, parms, wrk);
    break;
  case Y4M2_END:
    y4m2_emit_end(wrk->out);
    streak__free(wrk);
    break;
  }
}

y4m2_output *streak_hook(y4m2_output *out, jd_var *opt) {
  streak__work *wrk = y4m2_alloc(sizeof(streak__work));
  wrk->out = out;
  wrk->decay = jd_get_real(jd_rv(opt, "$.decay"));
  wrk->scale = 1;
  return y4m2_output_next(streak__callback, wrk);
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
