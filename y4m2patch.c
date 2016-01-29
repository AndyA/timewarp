/* y4m2patch.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <jd_pretty.h>

#include "yuv4mpeg2.h"

typedef struct {
  y4m2_parameters *parms;
  y4m2_output *out;
} output_ctx;

static y4m2_parameters *merge_parms(output_ctx *oc,
                                    const y4m2_parameters *parms) {
  y4m2_parameters *p = y4m2_clone_parms(parms);
  y4m2_merge_parms(p, oc->parms);
  return p;
}

static void callback(y4m2_reason reason, const y4m2_parameters *parms,
                     y4m2_frame *frame, void *ctx) {
  output_ctx *oc = (output_ctx *)ctx;
  y4m2_parameters *merged = NULL;

  switch (reason) {
  case Y4M2_START:
    merged = merge_parms(oc, parms);
    y4m2_emit_start(oc->out, merged);
    break;
  case Y4M2_FRAME:
    merged = merge_parms(oc, parms);
    y4m2_emit_frame(oc->out, merged, frame);
    break;
  case Y4M2_END:
    y4m2_emit_end(oc->out);
    break;
  }

  y4m2_free_parms(merged);
}

int main(int argc, char *argv[]) {
  output_ctx ctx;
  int argn;

  ctx.out = y4m2_output_file(stdout);
  ctx.parms = y4m2_new_parms();

  for (argn = 1; argn < argc; argn++) {
    y4m2_parameters *p = y4m2_new_parms();
    y4m2__parse_parms(p, argv[argn]);
    ctx.parms = y4m2_merge_parms(ctx.parms, p);
    y4m2_free_parms(p);
  }

  y4m2_output *out = y4m2_output_next(callback, &ctx);
  y4m2_parse(stdin, out);
  y4m2_free_parms(ctx.parms);

  return 0;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
