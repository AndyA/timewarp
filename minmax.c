/* minmax.c */

#include <stdint.h>
#include <string.h>

#include "filter.h"
#include "minmax.h"
#include "model.h"
#include "yuv4mpeg2.h"

typedef struct {
  unsigned phase;
  y4m2_frame *min;
  y4m2_frame *max;
  uint32_t *avg;
  y4m2_frame *tmp;
} minmax__work;

static void minmax__free(minmax__work *wrk) {
  if (wrk) {
    y4m2_release_frame(wrk->min);
    y4m2_release_frame(wrk->max);
    jd_free(wrk->avg);
    y4m2_release_frame(wrk->tmp);
    jd_free(wrk);
  }
}

static void minmax__frame_to_u32(uint32_t *out, const y4m2_frame *frame) {
  for (unsigned i = 0; i < frame->i.size; i++)
    out[i] = frame->buf[i];
}

static void minmax__start(filter *filt, const y4m2_parameters *parms) {
  if (!filt->ctx)
    filt->ctx = jd_alloc(sizeof(minmax__work));
  y4m2_emit_start(filt->out, parms);
}

static void minmax__frame(filter *filt, const y4m2_parameters *parms,
                          y4m2_frame *frame) {
  minmax__work *wrk = filt->ctx;

  unsigned frames = model_get_int(&filt->config, 10, "$.options.frames");

  double min = model_get_real(&filt->config, 0, "$.options.min");
  double max = model_get_real(&filt->config, 0, "$.options.max");
  double avg =
      model_get_real(&filt->config, 0, "$.options.average") / (double)frames;

  if (!wrk->avg) {
    wrk->min = y4m2_like_frame(frame);
    wrk->max = y4m2_like_frame(frame);
    wrk->avg = jd_alloc(sizeof(uint32_t) * frame->i.size);
    wrk->tmp = y4m2_like_frame(frame);
  }

  if (wrk->phase == 0) {
    memcpy(wrk->min->buf, frame->buf, frame->i.size);
    memcpy(wrk->max->buf, frame->buf, frame->i.size);
    minmax__frame_to_u32(wrk->avg, frame);
  } else {
    for (unsigned i = 0; i < frame->i.size; i++) {
      uint8_t sample = frame->buf[i];
      if (sample < wrk->min->buf[i])
        wrk->min->buf[i] = sample;
      if (sample > wrk->max->buf[i])
        wrk->max->buf[i] = sample;
      wrk->avg[i] += sample;
    }
  }

  if (++wrk->phase == frames) {
    wrk->tmp->sequence = frame->sequence;
    for (unsigned i = 0; i < wrk->tmp->i.size; i++) {
      double sample =
          wrk->min->buf[i] * min + wrk->max->buf[i] * max + wrk->avg[i] * avg;
      if (sample > 255)
        sample = 255;
      if (sample < 0)
        sample = 0;
      wrk->tmp->buf[i] = (uint8_t)sample;
    }
    y4m2_emit_frame(filt->out, parms, wrk->tmp);
    wrk->phase = 0;
  }
}

static void minmax__end(filter *filt) {
  y4m2_emit_end(filt->out);
  minmax__free(filt->ctx);
}

void minmax_register(void) {
  filter f = {
      .start = minmax__start, .frame = minmax__frame, .end = minmax__end};
  filter_register("minmax", &f);
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
