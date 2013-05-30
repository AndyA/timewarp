/* plotpipe.c */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include <jd_pretty.h>

#include "filter.h"
#include "filter_harness.h"
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

static jd_var *read_line(jd_var *out, FILE *f) {
  jd_set_array(out, 10);
  int c = getc(f);
  while (c != EOF && (jd_count(out) == 0 || c != '\n')) {
    if (c == '#') {
      while (c != EOF && c != '\n') c = getc(f);
    }
    else if (isdigit(c)) {
      unsigned v = 0;
      while (c != EOF && isdigit(c)) {
        v = v * 10 + c - '0';
        c = getc(f);
      }
      jd_set_int(jd_push(out, 1), v);
    }
    else if (isspace(c) || c == ',') {
      c = getc(f);
    }
    else {
      fprintf(stderr, "Syntax error");
      exit(1);
    }
  }
  return c == EOF ? NULL : out;
}

int main(int argc, char *argv[]) {
  scope {
    jd_var *config = jd_nav(10);
    jd_var capture = JD_INIT;
    jd_var *lp, *line = jd_nv();
    jd_var *frame = jd_nv();
    unsigned row = 0;

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

    y4m2_output *out = fh_get_output(&capture);
    abort(); // doesn't work currently
    y4m2_output *filt_out = filter_build(jd_nv(), out, config);

    while (lp = read_line(line, stdin), lp) {
      while (jd_count(lp) < 3) jd_set_int(jd_push(lp, 1), 0);
      fh_push_frame(filt_out,
                    jd_get_int(jd_get_idx(lp, 0)),
                    jd_get_int(jd_get_idx(lp, 1)),
                    jd_get_int(jd_get_idx(lp, 2)));

      while (jd_shift(&capture, 1, frame)) {
        printf("%u\t%ld\t%ld\t%ld\n", row++,
               jd_get_int(jd_get_idx(jd_get_idx(frame, 0), 0)),
               jd_get_int(jd_get_idx(jd_get_idx(frame, 1), 0)),
               jd_get_int(jd_get_idx(jd_get_idx(frame, 2), 0)));
      }
    }

    jd_release(&capture);
  }
  return 0;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
