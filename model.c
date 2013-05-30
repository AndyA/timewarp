/* model.c */

#include <jd_pretty.h>
#include <stdio.h>
#include <stdarg.h>

#include "model.h"

jd_var *model_vget(jd_var *v, jd_var *fallback, const char *path, va_list ap) {
  scope {
    jd_var *slot = jd_get_context(v, jd_vsprintf(jd_nv(), path, ap), 0);
    if (slot) JD_RETURN(slot);
  }
  return fallback;
}

jd_var *model_get(jd_var *v, jd_var *fallback, const char *path, ...) {
  va_list ap;
  va_start(ap, path);
  jd_var *slot = model_vget(v, fallback, path, ap);
  va_end(ap);
  return slot;
}

jd_int model_get_int(jd_var *v, jd_int fallback, const char *path, ...) {
  scope {
    va_list ap;
    va_start(ap, path);
    jd_var *slot = model_vget(v, jd_niv(fallback), path, ap);
    va_end(ap);
    JD_RETURN(jd_get_int(slot));
  }
  return 0;
}

double model_get_real(jd_var *v, double fallback, const char *path, ...) {
  scope {
    va_list ap;
    va_start(ap, path);
    jd_var *slot = model_vget(v, jd_nrv(fallback), path, ap);
    va_end(ap);
    JD_RETURN(jd_get_real(slot));
  }
  return 0;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
