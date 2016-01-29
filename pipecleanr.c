/* pipecleanr.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <jd_pretty.h>

#include "yuv4mpeg2.h"

typedef struct {
  y4m2_parameters *parms;
  y4m2_output *out;
} output_ctx;

static void callback(y4m2_reason reason, const y4m2_parameters *parms,
                     y4m2_frame *frame, void *ctx) {
  output_ctx *oc = (output_ctx *)ctx;
  switch (reason) {
  case Y4M2_START:
    if (!y4m2_equal_parms(oc->parms, parms)) {
      y4m2_free_parms(oc->parms);
      oc->parms = y4m2_clone_parms(parms);
      y4m2_emit_start(oc->out, parms);
    }
    break;
  case Y4M2_FRAME:
    y4m2_emit_frame(oc->out, parms, frame);
    break;
  case Y4M2_END:
    y4m2_emit_end(oc->out);
    break;
  }
}

int main(void) {
  output_ctx ctx;

  ctx.out = y4m2_output_file(stdout);
  ctx.parms = NULL;

  y4m2_output *out = y4m2_output_next(callback, &ctx);
  y4m2_parse(stdin, out);
  y4m2_free_parms(ctx.parms);

  return 0;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
