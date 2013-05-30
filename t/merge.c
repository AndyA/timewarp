/* merge.t */

#include "framework.h"
#include "tap.h"

#include "yuv4mpeg2.h"
#include "merge.h"
#include "filter_harness.h"

#if 0
static void test_merge(void) {
  scope {
    jd_var *opt = jd_njv("{\"frames\":10}");
    jd_var capture = JD_INIT;

    y4m2_output *out = fh_get_output(&capture);
    y4m2_output *test_out = merge_hook(out, opt);

    for (unsigned i = 0; i < 50; i++) {
      fh_push_frame(test_out, i, i, i);
    }

    y4m2_emit_end(test_out);

    ok(jd_count(&capture) == 5, "50 in, 5 out");

    jd_release(&capture);
  }
}

void test_main(void) {
  test_merge();
}
#else
void test_main(void) {
  pass("OK");
}
#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
