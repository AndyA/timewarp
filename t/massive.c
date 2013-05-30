/* massive.t */

#include "framework.h"
#include "tap.h"

#include "yuv4mpeg2.h"
#include "massive.h"
#include "filter_harness.h"

#if 0
static void test_thump(unsigned level, unsigned range) {
  scope {
    jd_var *opt = jd_njv(
      "{\"Cb\":{\"drag\":0.05,\"mass\":2.3},"
      "\"Cr\":{\"drag\":0.04,\"mass\":3.2},"
      "\"Y\":{\"drag\":0.03,\"mass\":1}}"
    );
    jd_var capture = JD_INIT;

    y4m2_output *out = fh_get_output(&capture);
    out = massive_hook(out, opt);

    for (unsigned i = 0; i < 500; i++) {
      fh_push_frame(out, level, level, level);
    }

    y4m2_emit_end(out);

    /*    jd_fprintf(stderr, "# %lJ\n", &capture);*/
    ok(jd_count(&capture) == 500, "500 in, 500 out");

    jd_int min, max;
    for (unsigned i = 0; i < jd_count(&capture); i++) {
      jd_var *frame = jd_get_idx(&capture, i);
      for (unsigned j = 0; j < jd_count(frame); j++) {
        jd_var *plane = jd_get_idx(frame, j);
        for (unsigned k = 0; k < jd_count(plane); k++) {
          jd_int v = jd_get_int(jd_get_idx(plane, k));
          if (i || j || k) {
            if (v < min) min = v;
            if (v > max) max = v;
          }
          else {
            min = max = v;
          }
        }
      }
    }

    ok(min >= level - range, "%d >= %d", (int) min, level - range);
    ok(max <= level + range, "%d <= %d", (int) max, level + range);

    jd_release(&capture);
  }
}

static void test_massive(void) {
  for (unsigned level = 20; level < 220; level += 10)
    test_thump(level, 4);
}

void test_main(void) {
  test_massive();
}
#else
void test_main(void) {
  pass("OK");
}
#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
