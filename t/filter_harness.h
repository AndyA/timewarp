/* filter_harness.h */

#ifndef FILTER_HARNESS_H_
#define FILTER_HARNESS_H_

#include "yuv4mpeg2.h"

#define FH_WIDTH     2
#define FH_HEIGHT    2

#define FH_Y_WIDTH   FH_WIDTH
#define FH_Y_HEIGHT  FH_HEIGHT
#define FH_Cb_WIDTH  (FH_Y_WIDTH/2)
#define FH_Cb_HEIGHT (FH_Y_HEIGHT/2)
#define FH_Cr_WIDTH  (FH_Y_WIDTH/2)
#define FH_Cr_HEIGHT (FH_Y_HEIGHT/2)

y4m2_output *fh_get_output(jd_var *capture);
void fh_push_frame(y4m2_output *out, uint8_t Y, uint8_t Cb, uint8_t Cr);

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
