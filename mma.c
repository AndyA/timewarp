/* mma.c */

#include <jd_pretty.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>

#include "mma.h"
#include "yuv4mpeg2.h"

static void mma__frame_to_u32(uint32_t *out, const y4m2_frame *frame) {
  for (unsigned i = 0; i < frame->i.size; i++)
    out[i] = frame->buf[i];
}

mma_accumulator *mma_new_accumulator(const y4m2_frame *frame) {
  mma_accumulator *acc = jd_alloc(sizeof(mma_accumulator));
  acc->min = y4m2_like_frame(frame);
  acc->max = y4m2_like_frame(frame);
  acc->avg = jd_alloc(sizeof(uint32_t) * frame->i.size);
  return acc;
}

void mma_free_accumulator(mma_accumulator *acc) {
  if (acc) {
    y4m2_release_frame(acc->min);
    y4m2_release_frame(acc->max);
    jd_free(acc->avg);
    jd_free(acc);
  }
}

mma_accumulator *mma_put_frame(mma_accumulator *acc, const y4m2_frame *frame) {
  if (acc == NULL)
    acc = mma_new_accumulator(frame);

  if (acc->phase == 0) {
    memcpy(acc->min->buf, frame->buf, frame->i.size);
    memcpy(acc->max->buf, frame->buf, frame->i.size);
    mma__frame_to_u32(acc->avg, frame);
  } else {
    for (unsigned i = 0; i < frame->i.size; i++) {
      uint8_t sample = frame->buf[i];
      if (sample < acc->min->buf[i])
        acc->min->buf[i] = sample;
      if (sample > acc->max->buf[i])
        acc->max->buf[i] = sample;
      acc->avg[i] += sample;
    }
  }

  acc->phase++;
  return acc;
}

mma_accumulator *mma_put_accumulator(mma_accumulator *acc,
                                     mma_accumulator *next) {
  if (next == NULL || next->phase == 0)
    return acc;

  if (acc == NULL)
    acc = mma_new_accumulator(next->min);

  if (acc->phase == 0) {
    memcpy(acc->min->buf, next->min->buf, next->min->i.size);
    memcpy(acc->max->buf, next->max->buf, next->max->i.size);
    memcpy(acc->avg, next->avg, sizeof(uint32_t) * next->max->i.size);
  } else {
    for (unsigned i = 0; i < acc->min->i.size; i++) {
      if (next->min->buf[i] < acc->min->buf[i])
        acc->min->buf[i] = next->min->buf[i];
      if (next->max->buf[i] > acc->max->buf[i])
        acc->max->buf[i] = next->max->buf[i];
      acc->avg[i] += next->avg[i];
    }
  }

  acc->phase += next->phase;
  return acc;
}

void mma_scale_average(mma_accumulator *acc) {
  if (acc->phase > 1) {
    for (unsigned i = 0; i < acc->min->i.size; i++)
      acc->avg[i] /= acc->phase;
    acc->phase = 1;
  }
}

void mma_clear_accumulator(mma_accumulator *acc) {
  if (acc)
    acc->phase = 0;
}

void mma_fill_frame(y4m2_frame *frame, const mma_accumulator *acc,
                    double min_weight, double max_weight, double avg_weight) {
  avg_weight /= (double)acc->phase;
  for (unsigned i = 0; i < frame->i.size; i++) {
    double sample = (double)acc->min->buf[i] * min_weight +
                    (double)acc->max->buf[i] * max_weight +
                    (double)acc->avg[i] * avg_weight;
    if (sample > 255)
      sample = 255;
    if (sample < 0)
      sample = 0;
    frame->buf[i] = (uint8_t)sample;
  }
}
