/* framework.h */

#ifndef __FRAMEWORK_H
#define __FRAMEWORK_H

#include <jd_pretty.h>
#include <stdio.h>

void test_main(void);

void tf_die(const char *msg, ...);
void *tf_alloc(size_t len);
char *tf_strdup(const char *s);
char *tf_base_dir(void);
char *tf_resource(const char *rel);
jd_var *tf_load_string(jd_var *out, FILE *f);
jd_var *tf_load_json(jd_var *out, FILE *f);
jd_var *tf_load_file(jd_var *out, const char *fn);
jd_var *tf_load_resource(jd_var *out, const char *fn);

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
