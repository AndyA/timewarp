/* minmax.c */

#include <stdint.h>
#include <string.h>

#include "filter.h"
#include "minmax.h"
#include "model.h"
#include "yuv4mpeg2.h"

typedef struct {
  unsigned phase;
  y4m2_frame *acc;
} minmax__work;

static void minmax__free(minmax__work *wrk) {
  if (wrk) {
    if (wrk->acc) y4m2_release_frame(wrk->acc);
    jd_free(wrk);
  }
}

static void minmax__start(filter *filt, const y4m2_parameters *parms) {
  if (!filt->ctx) filt->ctx = jd_alloc(sizeof(minmax__work));
  y4m2_emit_start(filt->out, parms);
}

static void minmax__frame(filter *filt, const y4m2_parameters *parms, y4m2_frame *frame) {
  minmax__work *wrk = filt->ctx;
  unsigned frames = model_get_int(&filt->config, 10, "$.options.frames");
  unsigned min = model_get_int(&filt->config, 0, "$.options.min");

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
    wrk->acc->sequence = frame->sequence;
    y4m2_emit_frame(filt->out, parms, wrk->acc);
    y4m2_release_frame(wrk->acc);
    wrk->acc = NULL;
    wrk->phase = 0;
  }
}

static void minmax__end(filter *filt) {
  y4m2_emit_end(filt->out);
  minmax__free(filt->ctx);
}

void minmax_register(void) {
  filter f = {
    .start = minmax__start,
    .frame = minmax__frame,
    .end = minmax__end
  };
  filter_register("minmax", &f);
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
