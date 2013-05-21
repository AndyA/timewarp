/* merge.h */

#ifndef MERGE_H_
#define MERGE_H_

#include "yuv4mpeg2.h"

typedef struct {
  unsigned long frames;
} merge_options;

y4m2_output *merge_hook(y4m2_output *out, const merge_options *opt);

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
