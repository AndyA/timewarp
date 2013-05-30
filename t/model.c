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
    jd_var *model = model_new(jd_nv(), jd_nsv(name));
    free(name);

    jd_var *data = model_load(model);
    ok(model_get_real(data, 0, "$.1.options.decay") == 0.9, "access field");
  }
}

static jd_var *resource_list(jd_var *out, const char const *res[]) {
  jd_set_array(out, 10);
  for (unsigned i = 0; res[i]; i++) {
    char *rname = tf_resource(res[i]);
    jd_set_string(jd_push(out, 1), rname);
    free(rname);
  }
  return out;
}

static void test_multi(void) {
  static const char *conf[] = {
    "data/timewarp.json",
    "data/massive.json",
    NULL
  };
  scope {
    jd_var *res = resource_list(jd_nv(), conf);
    jd_var *multi = model_multi_new(jd_nv(), res);
    jd_var *model = model_multi_load(jd_nv(), multi);
    /*    jd_fprintf(stderr, "# %lJ\n", multi);*/
    /*    jd_fprintf(stderr, "# %lJ\n", model);*/
    ok(model_get_real(model, 0, "$.3.options.Cr.mass") == 3.2, "access field");
  }
}

void test_main(void) {
  test_getters();
  test_load();
  test_multi();
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
