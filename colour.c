/* colour.c */

#include <stdint.h>

#include "colour.h"

#define SCF(x, s) ((x) * (s) / 256.0)
#define SCB(x, s) ((int32_t) (x) * (int32_t) ((s) * 256) / 65536)

static int32_t clamp_b(int32_t x, int32_t min, int32_t max) {
  if (x < min) return min;
  if (x > max) return max;
  return x;
}

void colour_f2b(const colour_floats *in, colour_bytes *out) {
  for (unsigned c = 0; c < cMAX; c++)
    out->c[c] = (uint8_t) clamp_b(in->c[c], 0, 255);
}

void colour_b2f(const colour_bytes *in, colour_floats *out) {
  for (unsigned c = 0; c < cMAX; c++)
    out->c[c] = in->c[c];
}

void colour_f_rgb2yuv(const colour_floats *in, colour_floats *out) {
  out->c[cY] = 16 +
               SCF(in->c[cR], 65.738) +
               SCF(in->c[cG], 129.057) +
               SCF(in->c[cB], 25.064);

  out->c[cCb] = 128 -
                SCF(in->c[cR], 37.945) -
                SCF(in->c[cG], 74.494) +
                SCF(in->c[cB], 112.439);

  out->c[cCr] = 128 +
                SCF(in->c[cR], 112.439) -
                SCF(in->c[cG], 94.154) -
                SCF(in->c[cB], 18.285);

  out->c[cA] = in->c[cA];
}

void colour_f_yuv2rgb(const colour_floats *in, colour_floats *out) {
  out->c[cR] = SCF(in->c[cY], 298.082) +
               SCF(in->c[cCr], 408.583) -
               222.921;

  out->c[cG] = SCF(in->c[cY], 298.082) -
               SCF(in->c[cCb], 100.291) -
               SCF(in->c[cCr], 208.120) +
               135.576;

  out->c[cB] = SCF(in->c[cY], 298.082) +
               SCF(in->c[cCb], 516.412) -
               276.836;

  out->c[cA] = in->c[cA];
}

void colour_b_rgb2yuv(const colour_bytes *in, colour_bytes *out) {
  out->c[cY] = clamp_b(16 +
                       SCB(in->c[cR], 65.738) +
                       SCB(in->c[cG], 129.057) +
                       SCB(in->c[cB], 25.064), 16, 235);

  out->c[cCb] = clamp_b(128 -
                        SCB(in->c[cR], 37.945) -
                        SCB(in->c[cG], 74.494) +
                        SCB(in->c[cB], 112.439), 16, 240);

  out->c[cCr] = clamp_b(128 +
                        SCB(in->c[cR], 112.439) -
                        SCB(in->c[cG], 94.154) -
                        SCB(in->c[cB], 18.285), 16, 240);

  out->c[cA] = in->c[cA];
}

void colour_b_yuv2rgb(const colour_bytes *in, colour_bytes *out) {
  out->c[cR] = clamp_b(SCB(in->c[cY], 298.082) +
                       SCB(in->c[cCr], 408.583) -
                       222.921, 0, 255);

  out->c[cG] = clamp_b(SCB(in->c[cY], 298.082) -
                       SCB(in->c[cCb], 100.291) -
                       SCB(in->c[cCr], 208.120) +
                       135.576, 0, 255);

  out->c[cB] = clamp_b(SCB(in->c[cY], 298.082) +
                       SCB(in->c[cCb], 516.412) -
                       276.836, 0, 255);

  out->c[cA] = in->c[cA];
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
