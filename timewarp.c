/* timewarp.c */

#include <stdio.h>

#include "yuv4mpeg2.h"

int main(void) {
  y4m2_output *out = y4m2_output_file(stdout);
  y4m2_parse(stdin, out);
  y4m2_free_output(out);
  return 0;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
