/* stretch.c */

#include <stdint.h>
#include <string.h>
#include <math.h>

#include <jd_pretty.h>

#include "average.h"
#include "filter.h"
#include "stretch.h"
#include "util.h"
#include "yuv4mpeg2.h"

typedef struct {
  y4m2_frame *prev, *tmp;
  jd_var config;
} stretch__work;

static void stretch__free(stretch__work *wrk) {
  if (wrk) {
    y4m2_release_frame(wrk->prev);
    y4m2_release_frame(wrk->tmp);
    jd_release(&wrk->config);
    y4m2_free(wrk);
  }
}

static void *stretch__configure(void *ctx, jd_var *config) {
  if (!ctx) ctx = y4m2_alloc(sizeof(stretch__work));
  stretch__work *wrk = ctx;
  jd_assign(&wrk->config, config);
  return ctx;
}

static void stretch__start(void *ctx, y4m2_output *out,
                         const y4m2_parameters *parms) {
  (void) ctx;
  y4m2_emit_start(out, parms);
}

static void stretch__frame(void *ctx, y4m2_output *out,
                         const y4m2_parameters *parms,
                         y4m2_frame *frame) {
  stretch__work *wrk = ctx;
  unsigned frames = util_get_int(jd_rv(&wrk->config, "$.frames"), 10);

  if (wrk->prev) {
    if (!wrk->tmp) wrk->tmp = y4m2_like_frame(frame);

    for (unsigned phase = 0; phase < frames; phase++) {
      uint8_t *fp = frame->buf;
      uint8_t *pp = wrk->prev->buf;
      uint8_t *tp = wrk->tmp->buf;

      uint32_t fw = (phase + 1);
      uint32_t pw = (frames - fw);
      uint32_t tw = (fw + pw);

      for (unsigned i = 0; i < frame->i.size; i++)
        *tp++ = (*pp++ * pw + *fp++ * fw) / tw;
      y4m2_emit_frame(out, parms, wrk->tmp);
    }

    y4m2_release_frame(wrk->prev);
  }
  else {
    y4m2_emit_frame(out, parms, frame);
  }

  wrk->prev = y4m2_retain_frame(frame);
}

static void stretch__end(void *ctx, y4m2_output *out) {
  y4m2_emit_end(out);
  stretch__free(ctx);
}

void stretch_register(void) {
  filter f = {
    .configure = stretch__configure,
    .start = stretch__start,
    .frame = stretch__frame,
    .end = stretch__end
  };
  filter_register("stretch", &f);
}


/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
