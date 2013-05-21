/* merge.c */

#include "merge.h"
#include "yuv4mpeg2.h"

typedef struct {
  y4m2_output *out;
} merge__work;

static void merge__callback(y4m2_reason reason,
                            const y4m2_parameters *parms,
                            y4m2_frame *frame,
                            void *ctx) {
  merge__work *wrk = (merge__work *) ctx;
  switch (reason) {
  case Y4M2_START:
    y4m2_emit_start(wrk->out, parms);
    break;
  case Y4M2_FRAME:
    y4m2_emit_frame(wrk->out, parms, frame);
    break;
  case Y4M2_END:
    y4m2_emit_end(wrk->out);
    y4m2_free(ctx);
    break;
  }
}

y4m2_output *merge_hook(y4m2_output *out) {
  merge__work *wrk = y4m2_alloc(sizeof(merge__work));
  wrk->out = out;
  return y4m2_output_next(merge__callback, wrk);
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
