/* pass.c */

#include <jd_pretty.h>

#include "pass.h"
#include "yuv4mpeg2.h"

typedef struct {
  y4m2_output *out;
} pass__work;

static void pass__callback(y4m2_reason reason,
                           const y4m2_parameters *parms, y4m2_frame *frame, void *ctx) {
  pass__work *wrk = (pass__work *) ctx;
  switch (reason) {
  case Y4M2_START:
    y4m2_emit_start(wrk->out, parms);
    break;
  case Y4M2_FRAME:
    y4m2_emit_frame(wrk->out, parms, frame);
    break;
  case Y4M2_END:
    y4m2_emit_end(wrk->out);
    jd_free(ctx);
    break;
  }
}

y4m2_output *pass_hook(y4m2_output *out) {
  pass__work *wrk = jd_alloc(sizeof(pass__work));
  wrk->out = out;
  return y4m2_output_next(pass__callback, wrk);
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
