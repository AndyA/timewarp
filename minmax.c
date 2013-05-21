/* minmax.c */

#include <stdint.h>
#include <string.h>

#include "minmax.h"
#include "yuv4mpeg2.h"

typedef struct {
  minmax_options opt;
  y4m2_output *out;
  unsigned phase;
  y4m2_frame *acc;
} minmax__work;

static void minmax__free(minmax__work *wrk) {
  if (wrk) {
    if (wrk->acc) y4m2_release_frame(wrk->acc);
    y4m2_free(wrk);
  }
}

static void minmax__frame(y4m2_frame *frame, const y4m2_parameters *parms, minmax__work *wrk) {
  if (wrk->acc) {
    if (wrk->opt.min) {
      for (unsigned i = 0; i < frame->i.size; i++) {
        if (frame->buf[i] < wrk->acc->buf[i])
          wrk->acc->buf[i] = frame->buf[i];
      }
    }
    else {
      for (unsigned i = 0; i < frame->i.size; i++) {
        if (frame->buf[i] > wrk->acc->buf[i])
          wrk->acc->buf[i] = frame->buf[i];
      }
    }
  }
  else {
    wrk->acc = y4m2_retain_frame(frame);
  }

  if (++wrk->phase == wrk->opt.frames) {
    y4m2_emit_frame(wrk->out, parms, wrk->acc);
    y4m2_release_frame(wrk->acc);
    wrk->acc = NULL;
    wrk->phase = 0;
  }
}

static void minmax__callback(y4m2_reason reason,
                             const y4m2_parameters *parms,
                             y4m2_frame *frame,
                             void *ctx) {
  minmax__work *wrk = (minmax__work *) ctx;
  switch (reason) {
  case Y4M2_START:
    y4m2_emit_start(wrk->out, parms);
    break;
  case Y4M2_FRAME:
    minmax__frame(frame, parms, wrk);
    break;
  case Y4M2_END:
    y4m2_emit_end(wrk->out);
    minmax__free(wrk);
    break;
  }
}

y4m2_output *minmax_hook(y4m2_output *out, const minmax_options *opt) {
  minmax__work *wrk = y4m2_alloc(sizeof(minmax__work));
  wrk->opt = *opt;
  wrk->out = out;
  return y4m2_output_next(minmax__callback, wrk);
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
