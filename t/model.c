/* basic.t */

#include <jd_pretty.h>

#include "framework.h"
#include "model.h"
#include "tap.h"

static void test_getters(void) {
  scope {
    jd_var *m = tf_load_resource(jd_nv(), "data/timewarp.json");
    /*    jd_fprintf(stderr, "# %lJ", m);*/

    ok(model_get_real(m, 0, "$.1.options.decay") == 0.9, "access field");
    ok(model_get_real(m, 1.23, "$.0.options.pi") == 1.23, "access default");
  }
}

void test_main(void) {
  test_getters();
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
