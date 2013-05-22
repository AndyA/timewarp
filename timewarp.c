/* timewarp.c */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <jd_pretty.h>

#include "yuv4mpeg2.h"
#include "filter.h"

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
    y4m2_output *filt_out = filter_build(out, config);

    y4m2_parse(stdin, filt_out);
    y4m2_free_output(out);
  }
  return 0;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
