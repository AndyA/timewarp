/* streak.c */

#include <stdint.h>
#include <string.h>
#include <math.h>

#include <jd_pretty.h>

#include "filter.h"
#include "model.h"
#include "streak.h"
#include "yuv4mpeg2.h"

typedef struct {
  double *acc;
  double scale;
} streak__work;

static void streak__free(streak__work *wrk) {
  if (wrk) {
    jd_free(wrk->acc);
    jd_free(wrk);
  }
}

static void streak__start(filter *filt, const y4m2_parameters *parms) {
  if (!filt->ctx) filt->ctx = jd_alloc(sizeof(streak__work));
  y4m2_emit_start(filt->out, parms);
}

static void streak__frame(filter *filt, const y4m2_parameters *parms, y4m2_frame *frame) {
  streak__work *wrk = filt->ctx;
  double decay = model_get_real(&filt->config, 0, "$.options.decay");

  if (!wrk->acc) wrk->acc = jd_alloc(frame->i.size * sizeof(double));

  for (unsigned i = 0; i < frame->i.size; i++) {
    wrk->acc[i] = wrk->acc[i] * decay + frame->buf[i];
    frame->buf[i] = (uint8_t)(wrk->acc[i] / wrk->scale);
  }

  wrk->scale = wrk->scale * decay + 1;

  y4m2_emit_frame(filt->out, parms, frame);
}

static void streak__end(filter *filt) {
  y4m2_emit_end(filt->out);
  streak__free(filt->ctx);
}

void streak_register(void) {
  filter f = {
    .start = streak__start,
    .frame = streak__frame,
    .end = streak__end
  };
  filter_register("streak", &f);
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
