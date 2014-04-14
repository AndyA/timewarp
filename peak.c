/* peak.c */

#include <stdint.h>
#include <string.h>
#include <math.h>

#include <jd_pretty.h>

#include "filter.h"
#include "model.h"
#include "peak.h"
#include "yuv4mpeg2.h"

#define SHIFT 23

typedef struct {
  double *acc;
  double scale[Y4M2_N_PLANE];;
} peak__work;

static void peak__free(peak__work *wrk) {
  if (wrk) {
    jd_free(wrk->acc);
    jd_free(wrk);
  }
}

static void peak__start(filter *filt, const y4m2_parameters *parms) {
  if (!filt->ctx) filt->ctx = jd_alloc(sizeof(peak__work));
  y4m2_emit_start(filt->out, parms);
}

static const char *plane_key[] = { "Y", "Cb", "Cr" };

static void peak__frame(filter *filt, const y4m2_parameters *parms, y4m2_frame *frame) {
  peak__work *wrk = filt->ctx;

  if (!wrk->acc) {
    wrk->acc = jd_alloc(frame->i.size * sizeof(double));
    for (unsigned p = 0; p < Y4M2_N_PLANE; p++)
      wrk->scale[p] = 1;
  }

  uint8_t *fp = frame->buf;
  double *ap = wrk->acc;
  double *sp = wrk->scale;

  for (unsigned p = 0; p < Y4M2_N_PLANE; p++) {
    if (model_get_int(&filt->config, 0, "$.options.%s.disabled", plane_key[p])) {
      ap += frame->i.plane[p].size;
      fp += frame->i.plane[p].size;
      sp++;
    }
    else {
      double decay = model_get_real(&filt->config, 0,
                                    "$.options.%s.decay", plane_key[p]);
      double scale = *sp;
      for (unsigned i = 0; i < frame->i.plane[p].size; i++) {
        double sample = *fp;
        double next = (*ap = *ap * decay + sample) / scale;
        ap++;
        *fp++ = (uint8_t)(sample > next ? sample : next);
      }
      *sp++ = scale * decay + 1;
    }
  }

  y4m2_emit_frame(filt->out, parms, frame);
}

static void peak__end(filter *filt) {
  y4m2_emit_end(filt->out);
  peak__free(filt->ctx);
}

void peak_register(void) {
  filter f = {
    .start = peak__start,
    .frame = peak__frame,
    .end = peak__end
  };
  filter_register("peak", &f);
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
