/* timewarp.c */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <jd_pretty.h>

#include "yuv4mpeg2.h"
#include "merge.h"
#include "minmax.h"
#include "streak.h"

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

static y4m2_output *filter_hook(const char *name, y4m2_output *out, jd_var *opt) {
/*  jd_fprintf(stderr, "Configuring %s with %lJ\n", name, opt);*/
  if (!strcmp("streak", name)) return streak_hook(out, opt);
  if (!strcmp("merge", name)) return merge_hook(out, opt);
  if (!strcmp("minmax", name)) return minmax_hook(out, opt);
  fprintf(stderr, "Unknown filter: %s\n", name);
  exit(1);
}

int main(int argc, char *argv[]) {
  scope {
    jd_var *config = jd_nav(10);

    for (int i = 1; i < argc; i++) {
      FILE *f = fopen(argv[i], "r");
      if (!f) {
        fprintf(stderr, "Can't read %s: %m\n", argv[i]);
        exit(1);
      }
      jd_var *cf = load_json(jd_nv(), f);
      jd_append(config, cf);
      fclose(f);
    }

    y4m2_output *out = y4m2_output_file(stdout);
    y4m2_output *last_out = out;

    for (int i = jd_count(config); --i >= 0;) {
      jd_var *filt = jd_get_idx(config, i);
      last_out = filter_hook(jd_bytes(jd_lv(filt, "$.filter"), NULL),
                             last_out, jd_lv(filt, "$.options"));
    }

    y4m2_parse(stdin, last_out);
    y4m2_free_output(out);
  }
  return 0;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
