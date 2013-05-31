/* stretch.c */

#include <stdint.h>
#include <string.h>
#include <math.h>

#include <jd_pretty.h>

#include "average.h"
#include "filter.h"
#include "stretch.h"
#include "model.h"
#include "yuv4mpeg2.h"

typedef struct {
  y4m2_frame *prev, *tmp;
} stretch__work;

static void stretch__free(stretch__work *wrk) {
  if (wrk) {
    y4m2_release_frame(wrk->prev);
    y4m2_release_frame(wrk->tmp);
    jd_free(wrk);
  }
}

static void stretch__start(filter *filt, const y4m2_parameters *parms) {
  if (!filt->ctx) filt->ctx = jd_alloc(sizeof(stretch__work));
  y4m2_emit_start(filt->out, parms);
}

static void stretch__frame(filter *filt, const y4m2_parameters *parms, y4m2_frame *frame) {
  stretch__work *wrk = filt->ctx;
  unsigned frames = model_get_int(&filt->config, 10, "$.options.frames");

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
      wrk->tmp->sequence = frame->sequence;
      y4m2_emit_frame(filt->out, parms, wrk->tmp);
    }

    y4m2_release_frame(wrk->prev);
  }
  else {
    y4m2_emit_frame(filt->out, parms, frame);
  }

  wrk->prev = y4m2_retain_frame(frame);
}

static void stretch__end(filter *filt) {
  y4m2_emit_end(filt->out);
  stretch__free(filt->ctx);
}

void stretch_register(void) {
  filter f = {
    .start = stretch__start,
    .frame = stretch__frame,
    .end = stretch__end
  };
  filter_register("stretch", &f);
}


/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
