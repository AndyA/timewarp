/* filter.h */

#ifndef FILTER_H_
#define FILTER_H_

#include <jd_pretty.h>

#include "yuv4mpeg2.h"

typedef void *(*filter_cb_configure)(void *ctx, jd_var *conf);
typedef void (*filter_cb_start)(void *ctx, y4m2_output *out,
                                const y4m2_parameters *parms);
typedef void (*filter_cb_frame)(void *ctx, y4m2_output *out,
                                const y4m2_parameters *parms,
                                y4m2_frame *frame);
typedef void (*filter_cb_end)(void *ctx, y4m2_output *out);

typedef struct {
  filter_cb_configure configure;
  filter_cb_start start;
  filter_cb_frame frame;
  filter_cb_end end;
  void *ctx;
  y4m2_output *out;
} filter;

void filter_init(void);
y4m2_output *filter_hook(const char *name, y4m2_output *out, jd_var *opt);
y4m2_output *filter_build(y4m2_output *out, jd_var *config);
void filter_register(const char *name, filter *f);

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
