/* mma.h */

#ifndef MMA_H_
#define MMA_H_

#include <stdint.h>

#include "yuv4mpeg2.h"

typedef struct {
  unsigned phase;
  y4m2_frame *min;
  y4m2_frame *max;
  uint32_t *avg;
} mma_accumulator;

mma_accumulator *mma_new_accumulator(const y4m2_frame *frame);
void mma_free_accumulator(mma_accumulator *acc);
mma_accumulator *mma_put_frame(mma_accumulator *acc, const y4m2_frame *frame);
mma_accumulator *mma_put_accumulator(mma_accumulator *acc,
                                     const mma_accumulator *next);
void mma_scale_average(mma_accumulator *acc);
void mma_clear_accumulator(mma_accumulator *acc);
void mma_fill_frame(y4m2_frame *frame, const mma_accumulator *acc,
                    double min_weight, double max_weight, double avg_weight);

#endif
