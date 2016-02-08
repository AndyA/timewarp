/* ripple.c */

#include <math.h>
#include <stdint.h>
#include <string.h>

#include <jd_pretty.h>

#include "filter.h"
#include "model.h"
#include "ripple.h"
#include "yuv4mpeg2.h"

typedef struct {
  double *acc;
  double scale;
} ripple__work;

static void ripple__free(ripple__work *wrk) {
  if (wrk) {
    jd_free(wrk->acc);
    jd_free(wrk);
  }
}

static void ripple__start(filter *filt, const y4m2_parameters *parms) {
  if (!filt->ctx)
    filt->ctx = jd_alloc(sizeof(ripple__work));
  y4m2_emit_start(filt->out, parms);
}

static void ripple__frame(filter *filt, const y4m2_parameters *parms,
                          y4m2_frame *frame) {
  ripple__work *wrk = filt->ctx;
  double decay = model_get_real(&filt->config, 0, "$.options.decay");

  if (!wrk->acc)
    wrk->acc = jd_alloc(frame->i.size * sizeof(double));

  for (unsigned i = 0; i < frame->i.size; i++) {
    wrk->acc[i] = wrk->acc[i] * decay + frame->buf[i];
    frame->buf[i] = (uint8_t)(wrk->acc[i] / wrk->scale);
  }

  wrk->scale = wrk->scale * decay + 1;

  y4m2_emit_frame(filt->out, parms, frame);
}

static void ripple__end(filter *filt) {
  y4m2_emit_end(filt->out);
  ripple__free(filt->ctx);
}

void ripple_register(void) {
  filter f = {
      .start = ripple__start, .frame = ripple__frame, .end = ripple__end};
  filter_register("ripple", &f);
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
