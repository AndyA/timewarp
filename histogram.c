/* histogram.c */

#include <stdint.h>
#include <string.h>
#include <math.h>

#include <jd_pretty.h>

#include "filter.h"
#include "model.h"
#include "histogram.h"
#include "hist.h"
#include "yuv4mpeg2.h"

static void histogram__start(filter *filt, const y4m2_parameters *parms) {
  y4m2_emit_start(filt->out, parms);
}

static const char *plane_key[] = { "Y", "Cb", "Cr" };

static void histogram__count(uint8_t *buf, size_t size, uint32_t *count) {
  memset(count, 0, 256 * sizeof(uint32_t));
  for (unsigned i = 0; i < size; i++)
    count[buf[i]]++;
}

static void histogram__scale_map(double *map, size_t size,
                                 uint8_t min, uint8_t max, uint8_t *out) {
  double mmin = map[0], mmax = map[size - 1];
  for (unsigned i = 0; i < size; i++)
    out[i] = (map[i] - mmin) * (max - min) / (mmax - mmin) + min;
}

static void histogram__frame(filter *filt, const y4m2_parameters *parms, y4m2_frame *frame) {
  uint32_t hist[256];
  double fwd_map[256];
  double rev_map[256];
  uint8_t xmap[256];

  uint8_t *fp = frame->buf;
  for (unsigned p = 0; p < Y4M2_N_PLANE; p++) {
    if (model_get_int(&filt->config, 0, "$.options.%s.disabled", plane_key[p])) {
      fp += frame->i.plane[p].size;
    }
    else {
      histogram__count(fp, frame->i.plane[p].size, hist);
      hist_map(hist, 256, fwd_map);
      hist_inverse(fwd_map, rev_map, 256);
      histogram__scale_map(rev_map, 256,
                           16, p == Y4M2_Y_PLANE ? 235 : 240,
                           xmap);
      for (unsigned i = 0; i < frame->i.plane[p].size; i++) {
        *fp = xmap[*fp];
        fp++;
      }
    }
  }

  y4m2_emit_frame(filt->out, parms, frame);
}

static void histogram__end(filter *filt) {
  y4m2_emit_end(filt->out);
}

void histogram_register(void) {
  filter f = {
    .start = histogram__start,
    .frame = histogram__frame,
    .end = histogram__end
  };
  filter_register("histogram", &f);
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
