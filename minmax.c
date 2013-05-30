/* minmax.c */

#include <stdint.h>
#include <string.h>

#include "filter.h"
#include "minmax.h"
#include "util.h"
#include "yuv4mpeg2.h"

typedef struct {
  unsigned phase;
  y4m2_frame *acc;
  jd_var config;
} minmax__work;

static void minmax__free(minmax__work *wrk) {
  if (wrk) {
    if (wrk->acc) y4m2_release_frame(wrk->acc);
    jd_release(&wrk->config);
    y4m2_free(wrk);
  }
}

static void *minmax__configure(void *ctx, jd_var *config) {
  if (!ctx) ctx = y4m2_alloc(sizeof(minmax__work));
  minmax__work *wrk = ctx;
  jd_assign(&wrk->config, config);
  return ctx;
}

static void minmax__start(void *ctx, y4m2_output *out,
                          const y4m2_parameters *parms) {
  (void) ctx;
  y4m2_emit_start(out, parms);
}

static void minmax__frame(void *ctx, y4m2_output *out,
                          const y4m2_parameters *parms,
                          y4m2_frame *frame) {
  minmax__work *wrk = ctx;
  unsigned frames = util_get_int(jd_rv(&wrk->config, "$.frames"), 10);
  unsigned min = util_get_int(jd_rv(&wrk->config, "$.min"), 0);

  if (wrk->acc) {
    if (min) {
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

  if (++wrk->phase == frames) {
    y4m2_emit_frame(out, parms, wrk->acc);
    y4m2_release_frame(wrk->acc);
    wrk->acc = NULL;
    wrk->phase = 0;
  }
}

static void minmax__end(void *ctx, y4m2_output *out) {
  y4m2_emit_end(out);
  minmax__free(ctx);
}

void minmax_register(void) {
  filter f = {
    .configure = minmax__configure,
    .start = minmax__start,
    .frame = minmax__frame,
    .end = minmax__end
  };
  filter_register("minmax", &f);
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
