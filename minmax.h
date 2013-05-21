/* minmax.h */

#ifndef MINMAX_H_
#define MINMAX_H_

#include "yuv4mpeg2.h"

typedef struct {
  unsigned long frames;
  unsigned int min;
} minmax_options;

y4m2_output *minmax_hook(y4m2_output *out, const minmax_options *opt);

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
