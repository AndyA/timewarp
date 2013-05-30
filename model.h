/* model.h */

#ifndef MODEL_H_
#define MODEL_H_

#include <jd_pretty.h>

jd_var *model_vget(jd_var *v, jd_var *fallback, const char *path, va_list ap);
jd_var *model_get(jd_var *v, jd_var *fallback, const char *path, ...);
jd_int model_get_int(jd_var *v, jd_int fallback, const char *path, ...);
double model_get_real(jd_var *v, double fallback, const char *path, ...);

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
