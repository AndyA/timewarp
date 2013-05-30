/* timewarp.c */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <jd_pretty.h>

#include "filter.h"
#include "massive.h"
#include "merge.h"
#include "minmax.h"
#include "model.h"
#include "streak.h"
#include "stretch.h"
#include "yuv4mpeg2.h"

static void setup(void) {
  filter_init();

  /* add filters here */
  merge_register();
  massive_register();
  minmax_register();
  streak_register();
  stretch_register();
}

static jd_var *build_model(jd_var *out, int nc, char *cfg[]) {
  scope {
    jd_var *cf = jd_nav(nc);
    for (int i = 0; i < nc; i++)
      jd_set_string(jd_push(cf, 1), cfg[i]);
    model_multi_new(out, cf);
  }
  return out;
}

int main(int argc, char *argv[]) {
  scope {
    setup();
    jd_var *model = build_model(jd_nv(), argc - 1, argv + 1);
    jd_var *config = model_multi_load(jd_nv(), model);

    y4m2_output *out = y4m2_output_file(stdout);
    y4m2_output *filt_out = filter_build(out, config);

    y4m2_parse(stdin, filt_out);
    y4m2_free_output(out);
  }
  return 0;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
