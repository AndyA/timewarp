/* filter.h */

#ifndef FILTER_H_
#define FILTER_H_

#include "yuv4mpeg2.h"

y4m2_output *filter_hook(const char *name, y4m2_output *out, jd_var *opt);
y4m2_output *filter_build(y4m2_output *out, jd_var *config);

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
