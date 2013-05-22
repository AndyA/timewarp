/* basic.t */

#include "framework.h"
#include "tap.h"
#include "util.h"

static void contrived(double *out, double a, double b, double c) {
  out[0] = a;
  out[1] = b;
  out[2] = c;
}

static double get_real(jd_var *v, double fallback) {
  double d = util_get_real(v, fallback);
/*  jd_fprintf(stderr, "# util_get_real(%V, %g) -> %g\n", v, fallback, d);*/
  return d;
}

static void test_get_real(void) {
  scope {
    ok(util_get_real(NULL, 1) == 1, "fallback");

    jd_var *v = jd_njv("{\"test_decay\":0.9}");
    double out[3];
    const char *ns = "test";

    contrived(out,
    get_real(jd_rv(v, "$.%s_decay", ns), 0),
    get_real(jd_rv(v, "$.%s_ow", ns), 1),
    get_real(jd_rv(v, "$.%s_nw", ns), 1));

    ok(out[0] == 0.9, "out[0] == 0.9");
    ok(out[1] == 1, "out[1] == 1");
    ok(out[2] == 1, "out[2] == 2");
  }
}

void test_main(void) {
  test_get_real();
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
