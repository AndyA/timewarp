/* streak.c */

#include <stdint.h>
#include <string.h>
#include <math.h>

#include <jd_pretty.h>

#include "filter.h"
#include "util.h"
#include "streak.h"
#include "yuv4mpeg2.h"

#define SHIFT 23

typedef struct {
  double *acc;
  double decay;
  double scale;
  jd_var config;
} streak__work;

static void streak__free(streak__work *wrk) {
  if (wrk) {
    y4m2_free(wrk->acc);
    jd_release(&wrk->config);
    y4m2_free(wrk);
  }
}

static void *streak__configure(void *ctx, jd_var *config) {
  if (!ctx) ctx = y4m2_alloc(sizeof(streak__work));
  streak__work *wrk = ctx;
  jd_assign(&wrk->config, config);
  return ctx;
}

static void streak__start(void *ctx, y4m2_output *out,
                          const y4m2_parameters *parms) {
  (void) ctx;
  y4m2_emit_start(out, parms);
}

static void streak__frame(void *ctx, y4m2_output *out,
                          const y4m2_parameters *parms,
                          y4m2_frame *frame) {
  streak__work *wrk = ctx;
  double decay = util_get_real(jd_rv(&wrk->config, "$.decay"), 10);

  if (!wrk->acc) wrk->acc = y4m2_alloc(frame->i.size * sizeof(double));

  for (unsigned i = 0; i < frame->i.size; i++) {
    wrk->acc[i] = wrk->acc[i] * decay + frame->buf[i];
    frame->buf[i] = (uint8_t)(wrk->acc[i] / wrk->scale);
  }

  wrk->scale = wrk->scale * decay + 1;

  y4m2_emit_frame(out, parms, frame);
}

static void streak__end(void *ctx, y4m2_output *out) {
  y4m2_emit_end(out);
  streak__free(ctx);
}

void streak_register(void) {
  filter f = {
    .configure = streak__configure,
    .start = streak__start,
    .frame = streak__frame,
    .end = streak__end
  };
  filter_register("streak", &f);
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
