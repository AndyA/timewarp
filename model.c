/* model.c */

#include <jd_pretty.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

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

jd_var *model_load_string(jd_var *out, FILE *f) {
  char buf[512];
  size_t got;

  jd_set_empty_string(out, 10000);
  while (got = fread(buf, 1, sizeof(buf), f), got)
    jd_append_bytes(out, buf, got);
  return out;
}

jd_var *model_load_json(jd_var *out, FILE *f) {
  jd_var json = JD_INIT;
  jd_from_json(out, model_load_string(&json, f));
  jd_release(&json);
  return out;
}

jd_var *model_load_file(jd_var *out, const char *fn) {
  FILE *fl = fopen(fn, "r");
  if (!fl) jd_throw("Can't read %s: %m\n", fn);
  jd_var *v = model_load_json(out, fl);
  fclose(fl);
  return v;
}

jd_var *model_new(jd_var *stash, const char *name) {
  scope jd_set_hash_with(stash, jd_nsv("name"), jd_nsv(name), NULL);
  return stash;
}

jd_var *model_load(jd_var *stash) {
  struct stat st;
  const char *name = jd_bytes(jd_get_ks(stash, "name", 0), NULL);
  jd_var *lm = jd_get_ks(stash, "last_modified", 1);
  if (stat(name, &st)) jd_throw("Can't stat %s: %m", name);
  if (!lm || jd_get_int(lm) < st.st_mtime) {
    model_load_file(jd_get_ks(stash, "model", 1), name);
    jd_set_int(lm, st.st_mtime);
  }

  return jd_get_ks(stash, "model", 0);
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
