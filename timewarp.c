/* timewarp.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <jd_pretty.h>

#include "filter.h"
#include "histogram.h"
#include "massive.h"
#include "merge.h"
#include "minmax.h"
#include "model.h"
#include "peak.h"
#include "streak.h"
#include "stretch.h"
#include "yuv4mpeg2.h"

static void setup(void) {
  filter_init();

  /* add filters here */
  histogram_register();
  massive_register();
  merge_register();
  minmax_register();
  peak_register();
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

static void help(void) { printf("Syntax: timewarp <config file>...\n"); }

int main(int argc, char *argv[]) {
  if (argc == 1 ||
      (argc == 2 && (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")))) {
    help();
    return 1;
  }
  scope {
    setup();
    jd_var *model = build_model(jd_nv(), argc - 1, argv + 1);
    jd_var *filters = jd_nv();

    y4m2_output *out = y4m2_output_file(stdout);
    y4m2_output *filt_out = filter_build(filters, out, model);

    y4m2_parse(stdin, filt_out);
    y4m2_free_output(out);
  }
  return 0;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
