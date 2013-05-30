/* util.c */

#include <jd_pretty.h>

#include "util.h"

jd_int util_get_int(jd_var *v, jd_int fallback) {
  return v ? jd_get_int(v) : fallback;
}

double util_get_real(jd_var *v, double fallback) {
  return v ? jd_get_real(v) : fallback;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
