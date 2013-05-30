/* basic.t */

#include <jd_pretty.h>
#include <stdlib.h>

#include "framework.h"
#include "model.h"
#include "tap.h"

static void test_getters(void) {
  scope {
    jd_var *m = tf_load_resource(jd_nv(), "data/timewarp.json");
    /*    jd_fprintf(stderr, "# %lJ\n", m);*/

    ok(model_get_real(m, 0, "$.1.options.decay") == 0.9, "access field");
    ok(model_get_real(m, 1.23, "$.0.options.pi") == 1.23, "access default");
  }
}

static void test_load(void) {
  scope {
    char *name = tf_resource("data/timewarp.json");
    jd_var *model = model_new(jd_nv(), name);
    free(name);

    jd_var *data = model_load(model);
    ok(model_get_real(data, 0, "$.1.options.decay") == 0.9, "access field");
  }
}

void test_main(void) {
  test_getters();
  test_load();
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
