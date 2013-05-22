/* util.h */

#ifndef UTIL_H_
#define UTIL_H_

#include <jd_pretty.h>

jd_int util_get_int(jd_var *v, jd_int fallback);
double util_get_real(jd_var *v, double fallback);

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
