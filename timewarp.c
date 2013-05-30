/* timewarp.c */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <jd_pretty.h>

#include "filter.h"
#include "massive.h"
#include "merge.h"
#include "minmax.h"
#include "streak.h"
#include "stretch.h"
#include "util.h"
#include "wobble.h"
#include "yuv4mpeg2.h"

static jd_var *load_string(jd_var *out, FILE *f) {
  char buf[512];
  size_t got;

  jd_set_empty_string(out, 10000);
  while (got = fread(buf, 1, sizeof(buf), f), got)
    jd_append_bytes(out, buf, got);
  return out;
}

static jd_var *load_json(jd_var *out, FILE *f) {
  jd_var json = JD_INIT;
  jd_from_json(out, load_string(&json, f));
  jd_release(&json);
  return out;
}

static void apply_patch(jd_var *conf, const char *patch) {
  scope {
    jd_var *pv = jd_nsv(patch);
    int eq = jd_find(pv, jd_nsv("="), 0);
    if (eq < 0) jd_throw("Bad patch: %V", pv);
    jd_var *path = jd_trim(jd_nv(), jd_substr(jd_nv(), pv, 0, eq));
    jd_var *value = jd_substr(jd_nv(), pv, eq + 1, jd_length(pv));
    jd_assign(jd_lv(conf, jd_bytes(path, NULL)), jd_from_json(jd_nv(), value));
  }
}

static void setup(void) {
  filter_init();

  /* add filters here */
  merge_register();
  massive_register();
  minmax_register();
  streak_register();
}

int main(int argc, char *argv[]) {
  scope {
    setup();
    jd_var *config = jd_nav(10);
    int i;

    for (i = 1; i < argc; i++) {
      if (argv[i][0] == '$') continue;
      FILE *f = fopen(argv[i], "r");
      if (!f) jd_throw("Can't read %s: %m\n", argv[i]);
      jd_var *cf = load_json(jd_nv(), f);
      jd_append(config, cf);
      fclose(f);
    }

    for (i = 1; i < argc; i++) {
      if (argv[i][0] != '$') continue;
      apply_patch(config, argv[i]);
    }

    y4m2_output *out = y4m2_output_file(stdout);
    y4m2_output *filt_out = filter_build(out, config);

    y4m2_parse(stdin, filt_out);
    y4m2_free_output(out);
  }
  return 0;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
