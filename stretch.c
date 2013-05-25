/* stretch.c */

#include <stdint.h>
#include <string.h>
#include <math.h>

#include <jd_pretty.h>

#include "yuv4mpeg2.h"
#include "average.h"
#include "stretch.h"
#include "util.h"

typedef struct {
  y4m2_output *out;
  unsigned frames;

  y4m2_frame *prev, *tmp;

} stretch__work;

static void stretch__free(stretch__work *wrk) {
  if (wrk) {
    y4m2_release_frame(wrk->prev);
    y4m2_release_frame(wrk->tmp);
    y4m2_free(wrk);
  }
}

static void stretch__frame(y4m2_frame *frame,
                           const y4m2_parameters *parms,
                           stretch__work *wrk) {


  if (wrk->prev) {
    if (!wrk->tmp) wrk->tmp = y4m2_like_frame(frame);

    for (unsigned phase = 0; phase < wrk->frames; phase++) {
      uint8_t *fp = frame->buf;
      uint8_t *pp = wrk->prev->buf;
      uint8_t *tp = wrk->tmp->buf;

      uint32_t fw = (phase + 1);
      uint32_t pw = (wrk->frames - fw);
      uint32_t tw = (fw + pw);

      for (unsigned i = 0; i < frame->i.size; i++)
        *tp++ = (*pp++ * pw + *fp++ * fw) / tw;
      y4m2_emit_frame(wrk->out, parms, wrk->tmp);
    }

    y4m2_release_frame(wrk->prev);
  }
  else {
    y4m2_emit_frame(wrk->out, parms, frame);
  }

  wrk->prev = y4m2_retain_frame(frame);
}

static void stretch__callback(y4m2_reason reason,
                              const y4m2_parameters *parms,
                              y4m2_frame *frame,
                              void *ctx) {
  stretch__work *wrk = (stretch__work *) ctx;
  switch (reason) {
  case Y4M2_START:
    y4m2_emit_start(wrk->out, parms);
    break;
  case Y4M2_FRAME:
    stretch__frame(frame, parms, wrk);
    break;
  case Y4M2_END:
    y4m2_emit_end(wrk->out);
    stretch__free(wrk);
    break;
  }
}

y4m2_output *stretch_hook(y4m2_output *out, jd_var *opt) {
  stretch__work *wrk = y4m2_alloc(sizeof(stretch__work));
  wrk->out = out;
  wrk->frames = util_get_int(jd_rv(opt, "$.frames"), 10);
  return y4m2_output_next(stretch__callback, wrk);
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
