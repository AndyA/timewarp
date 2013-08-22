/* framework.c */

#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "framework.h"
#include "tap.h"

void tf_die(const char *msg, ...) {
  va_list ap;
  va_start(ap, msg);
  vfprintf(stderr, msg, ap);
  fprintf(stderr, "\n");
  va_end(ap);
  exit(1);
}

void *tf_alloc(size_t len) {
  void *m = NULL;
  if (len == 0 || (m = calloc(1, len), !m))
    tf_die("Can't allocate %llu bytes", (unsigned long long) len);
  return m;
}

char *tf_strdup(const char *s) {
  if (!s) return NULL;
  size_t l = strlen(s);
  char *ns = tf_alloc(l + 1);
  memcpy(ns, s, l + 1);
  return ns;
}

static void clean_name(char *name) {
  size_t nl = strlen(name);
  while (name[0] == '.' && name[1] == '/') {
    memmove(name, name + 2, nl - 1);
    nl -= 2;
  }
  char *dot;
  while (dot = strstr(name, "/./"), dot) {
    memmove(dot, dot + 2, nl - 1 - (dot - name));
    nl -= 2;
  }
  while (dot = strstr(name, "/../"), dot) {
    if (dot == name) {
      memmove(name, name + 3, nl - 2);
      nl -= 3;
    }
    else {
      char *slash;
      for (slash = dot; slash-- != name && *slash != '/';)
        ;
      if (*slash == '/') {
        memmove(slash, dot + 3, nl - 2 - (dot - name));
        nl -= 3 + dot - slash;
      }
    }
  }
}

static char *abs_name(char *name) {
  if (!name) tf_die("name == NULL");
  char *nb;
  if (name[0] == '/') {
    nb = tf_strdup(name);
  }
  else {
    char cwd[1024];
    if (!getcwd(cwd, sizeof(cwd))) abort();
    size_t nlen = strlen(name);
    size_t clen = strlen(cwd);
    size_t len = clen + nlen + 2;
    nb = tf_alloc(len);
    memcpy(nb, cwd, clen);
    nb[clen] = '/';
    memcpy(nb + clen + 1, name, nlen + 1);
  }
  clean_name(nb);
  return nb;
}

static char *abs_path(char *name) {
  char *nb = abs_name(name);
  int i;
  for (i = strlen(nb); i > 0 && nb[i - 1] != '/'; i--)
    ;
  nb[i] = '\0';
  return nb;
}

static char *argv0 = NULL;
static char *base_dir = NULL;

static void free_base_dir(void) {
  free(base_dir);
}

char *tf_base_dir(void) {
  if (!base_dir) {
    base_dir = abs_path(argv0);
    atexit(free_base_dir);
  }
  return base_dir;
}

char *tf_resource(const char *rel) {
  char *nb;
  if (!rel) return NULL;
  if (*rel == '/') {
    nb = tf_strdup(rel);
  }
  else {
    char *bd = tf_base_dir();
    size_t bl = strlen(bd);
    size_t rl = strlen(rel);
    nb = tf_alloc(bl + rl + 1);
    memcpy(nb, bd, bl);
    memcpy(nb + bl, rel, rl + 1);
  }
  clean_name(nb);
  return nb;
}

jd_var *tf_load_string(jd_var *out, FILE *f) {
  char buf[512];
  size_t got;

  jd_set_empty_string(out, 10000);
  while (got = fread(buf, 1, sizeof(buf), f), got)
    jd_append_bytes(out, buf, got);
  return out;
}

jd_var *tf_load_json(jd_var *out, FILE *f) {
  jd_var json = JD_INIT;
  jd_from_json(out, tf_load_string(&json, f));
  jd_release(&json);
  return out;
}

jd_var *tf_load_file(jd_var *out, const char *fn) {
  FILE *fl = fopen(fn, "r");
  if (!fl) tf_die("Can't read %s: %m\n", fn);
  jd_var *v = tf_load_json(out, fl);
  fclose(fl);
  return v;
}

jd_var *tf_load_resource(jd_var *out, const char *fn) {
  char *res = tf_resource(fn);
  jd_var *v = tf_load_file(out, res);
  free(res);
  return v;
}

int main(int argc, char *argv[]) {
  int i, count = 1;
  const char *tc_env = NULL;
  argv0 = argv[0];

  if (argc > 1)
    count = atoi(argv[1]);
  else if (tc_env = getenv("TEST_COUNT"), tc_env)
    count = atoi(tc_env);

  for (i = 0; i < count; i++)
    test_main();

  done_testing();
  return 0;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
