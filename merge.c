/* merge.c */

#include <stdint.h>
#include <string.h>

#include <jd_pretty.h>

#include "filter.h"
#include "merge.h"
#include "model.h"
#include "yuv4mpeg2.h"

typedef struct {
  unsigned phase;
  uint32_t *avg;
} merge__work;

static void merge__free(merge__work *wrk) {
  if (wrk) {
    jd_free(wrk->avg);
    jd_free(wrk);
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

static void merge__start(filter *filt, const y4m2_parameters *parms) {
  if (!filt->ctx) filt->ctx = jd_alloc(sizeof(merge__work));
  y4m2_emit_start(filt->out, parms);
}

static void merge__frame(filter *filt, const y4m2_parameters *parms, y4m2_frame *frame) {
  merge__work *wrk = filt->ctx;
  unsigned frames = model_get_int(&filt->config, 10, "$.options.frames");
  if (!wrk->avg)
    wrk->avg = jd_alloc(frame->i.size * sizeof(uint32_t));
  merge__add(wrk->avg, frame);
  if (++wrk->phase == frames) {
    merge__scale(frame, wrk->avg, frames);
    y4m2_emit_frame(filt->out, parms, frame);
    memset(wrk->avg, 0, frame->i.size * sizeof(uint32_t));
    wrk->phase = 0;
  }
}

static void merge__end(filter *filt) {
  y4m2_emit_end(filt->out);
  merge__free(filt->ctx);
}

void merge_register(void) {
  filter f = {
    .start = merge__start,
    .frame = merge__frame,
    .end = merge__end
  };
  filter_register("merge", &f);
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
