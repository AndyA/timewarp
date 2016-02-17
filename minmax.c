/* minmax.c */

#include <stdint.h>
#include <string.h>

#include "filter.h"
#include "minmax.h"
#include "mma.h"
#include "model.h"
#include "yuv4mpeg2.h"

typedef struct {
  mma_accumulator **buckets;
  size_t nbuckets;
  unsigned cbucket;
  mma_accumulator *acc;
  y4m2_frame *tmp;
} minmax__work;

static void minmax__free(minmax__work *wrk) {
  if (wrk) {
    for (unsigned i = 0; i < wrk->nbuckets; i++)
      mma_free_accumulator(wrk->buckets[i]);
    jd_free(wrk->buckets);
    mma_free_accumulator(wrk->acc);
    y4m2_release_frame(wrk->tmp);
    jd_free(wrk);
  }
}

static void minmax__start(filter *filt, const y4m2_parameters *parms) {
  if (!filt->ctx) {
    unsigned buckets = model_get_int(&filt->config, 1, "$.options.buckets");
    minmax__work *wrk = jd_alloc(sizeof(minmax__work));
    filt->ctx = wrk;
  }
  y4m2_emit_start(filt->out, parms);
}

static void minmax__frame(filter *filt, const y4m2_parameters *parms,
                          y4m2_frame *frame) {
  minmax__work *wrk = filt->ctx;

  unsigned frames = model_get_int(&filt->config, 10, "$.options.frames");

  if (!wrk->buckets) {
    unsigned buckets = model_get_int(&filt->config, 1, "$.options.buckets");
    wrk->buckets = jd_alloc(sizeof(mma_accumulator *) * buckets);
    wrk->nbuckets = buckets;
  }

  wrk->buckets[wrk->cbucket] = mma_put_frame(wrk->buckets[wrk->cbucket], frame);

  if (wrk->buckets[wrk->cbucket]->phase == frames) {
    mma_clear_accumulator(wrk->acc);
    for (unsigned i = 0; i < wrk->nbuckets; i++)
      if (wrk->buckets[i])
        wrk->acc = mma_put_accumulator(wrk->acc, wrk->buckets[i]);

    if (!wrk->tmp)
      wrk->tmp = y4m2_like_frame(frame);

    double min = model_get_real(&filt->config, 0, "$.options.min");
    double max = model_get_real(&filt->config, 0, "$.options.max");
    double avg = model_get_real(&filt->config, 0, "$.options.average");

    mma_fill_frame(wrk->tmp, wrk->acc, min, max, avg);
    wrk->tmp->sequence = frame->sequence;
    y4m2_emit_frame(filt->out, parms, wrk->tmp);

    if (++wrk->cbucket == wrk->nbuckets)
      wrk->cbucket = 0;
    mma_clear_accumulator(wrk->buckets[wrk->cbucket]);
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
