/* model.h */

#ifndef MODEL_H_
#define MODEL_H_

#include <jd_pretty.h>

jd_var *model_vget(jd_var *v, jd_var *fallback, const char *path, va_list ap);
jd_var *model_get(jd_var *v, jd_var *fallback, const char *path, ...);
jd_int model_get_int(jd_var *v, jd_int fallback, const char *path, ...);
double model_get_real(jd_var *v, double fallback, const char *path, ...);

jd_var *model_load_string(jd_var *out, FILE *f);
jd_var *model_load_json(jd_var *out, FILE *f);
jd_var *model_load_file(jd_var *out, const char *fn);
jd_var *model_new(jd_var *stash, jd_var *name);
jd_var *model_load(jd_var *stash);

jd_var *model_multi_new(jd_var *stash, jd_var *names);
jd_var *model_multi_load(jd_var *out, jd_var *stash);

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
