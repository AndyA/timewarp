/* merge.c */

#include <stdint.h>
#include <string.h>

#include <jd_pretty.h>

#include "filter.h"
#include "merge.h"
#include "util.h"
#include "yuv4mpeg2.h"

typedef struct {
  y4m2_output *out;
  unsigned frames;
  unsigned phase;
  uint32_t *avg;
} merge__work;

static void merge__free(merge__work *wrk) {
  if (wrk) {
    y4m2_free(wrk->avg);
    y4m2_free(wrk);
  }
}

static void merge__add(uint32_t *avg, y4m2_frame *frame) {
  for (unsigned i = 0; i < frame->i.size; i++)
    avg[i] += frame->buf[i];
}

static void merge__scale(y4m2_frame *frame, uint32_t *avg, unsigned scale) {
  for (unsigned i = 0; i < frame->i.size; i++)
    frame->buf[i] = avg[i] / scale;
}

static void merge__next_frame(y4m2_frame *frame, const y4m2_parameters *parms, merge__work *wrk) {
  if (!wrk->avg)
    wrk->avg = y4m2_alloc(frame->i.size * sizeof(uint32_t));
  merge__add(wrk->avg, frame);
  if (++wrk->phase == wrk->frames) {
    merge__scale(frame, wrk->avg, wrk->frames);
    y4m2_emit_frame(wrk->out, parms, frame);
    memset(wrk->avg, 0, frame->i.size * sizeof(uint32_t));
    wrk->phase = 0;
  }
}

static void merge__callback(y4m2_reason reason,
                            const y4m2_parameters *parms,
                            y4m2_frame *frame,
                            void *ctx) {
  merge__work *wrk = (merge__work *) ctx;
  switch (reason) {
  case Y4M2_START:
    y4m2_emit_start(wrk->out, parms);
    break;
  case Y4M2_FRAME:
    merge__next_frame(frame, parms, wrk);
    break;
  case Y4M2_END:
    y4m2_emit_end(wrk->out);
    merge__free(wrk);
    break;
  }
}

static void *merge__configure(void *ctx, jd_var *config) {
  if (!ctx) ctx = y4m2_alloc(sizeof(merge__work));
  merge__work *wrk = ctx;
  wrk->frames = util_get_int(jd_rv(config, "$.frames"), 10);
  return ctx;
}

static void merge__start(void *ctx, y4m2_output *out,
                         const y4m2_parameters *parms) {
  (void) ctx;
  y4m2_emit_start(out, parms);
}

static void merge__frame(void *ctx, y4m2_output *out,
                         const y4m2_parameters *parms,
                         y4m2_frame *frame) {
  merge__work *wrk = ctx;
  if (!wrk->avg)
    wrk->avg = y4m2_alloc(frame->i.size * sizeof(uint32_t));
  merge__add(wrk->avg, frame);
  if (++wrk->phase == wrk->frames) {
    merge__scale(frame, wrk->avg, wrk->frames);
    y4m2_emit_frame(out, parms, frame);
    memset(wrk->avg, 0, frame->i.size * sizeof(uint32_t));
    wrk->phase = 0;
  }
}

static void merge__end(void *ctx, y4m2_output *out) {
  y4m2_emit_end(out);
  merge__free(ctx);
}

y4m2_output *merge_hook(y4m2_output *out, jd_var *opt) {
  merge__work *wrk = y4m2_alloc(sizeof(merge__work));
  wrk->out = out;
  wrk->frames = util_get_int(jd_rv(opt, "$.frames"), 10);
  return y4m2_output_next(merge__callback, wrk);
}

void merge_register(void) {
  filter f = {
    .configure = merge__configure,
    .start = merge__start,
    .frame = merge__frame,
    .end = merge__end
  };
  filter_register("merge", &f);
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
