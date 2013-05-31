/* colour.c */

#include <stdint.h>
#include <stdlib.h>

#include "colour.h"

#define MIN2(a, b)    ((a) < (b) ? (a) : (b))
#define MAX2(a, b)    ((a) > (b) ? (a) : (b))
#define MIN3(a, b, c) MIN2(MIN2(a, b), c)
#define MAX3(a, b, c) MAX2(MAX2(a, b), c)

#define SCF(x, s)     ((x) * (s) / 256.0)
#define SCB(x, s)     ((int32_t) (x) * (int32_t) ((s) * 256) / 65536)

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

/* doubles */

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

void colour_f_rgb2hsv(const colour_floats *in, colour_floats *out) {
  double min = MIN3(in->c[0], in->c[1], in->c[2]);
  double max = MAX3(in->c[0], in->c[1], in->c[2]);
  double d = max - min;
  out->c[cV] = max;
  out->c[cS] = (max == 0 ? 0 : d / max) * 255;

  double h = (0 == out->c[cS] ? 0
              : max == in->c[cR] ? 0 + (in->c[cG] - in->c[cB]) / d
              : max == in->c[cG] ? 2 + (in->c[cB] - in->c[cR]) / d
              :                    4 + (in->c[cR] - in->c[cG]) / d) * 256 / 6;

  if (h < 0) h += 256;
  out->c[cH] = h;
  out->c[cA] = in->c[cA];
}

void colour_f_hsv2rgb(const colour_floats *in, colour_floats *out) {
  if (in->c[cS] == 0) {
    out->c[cR] = out->c[cG] = out->c[cB] = in->c[cV];
  }
  else {
    double h = in->c[cH] * 6 / 256;
    int sextant = (int) h;
    double frac = h - sextant;
    double p = in->c[cV] * (1 - in->c[cS] / 255);
    double q = in->c[cV] * (1 - (in->c[cS] / 255 * frac));
    double t = in->c[cV] * (1 - (in->c[cS] / 255 * (1 - frac)));
    switch (sextant) {
    case 0:
      out->c[cR] = in->c[cV];
      out->c[cG] = t;
      out->c[cB] = p;
      break;
    case 1:
      out->c[cR] = q;
      out->c[cG] = in->c[cV];
      out->c[cB] = p;
      break;
    case 2:
      out->c[cR] = p;
      out->c[cG] = in->c[cV];
      out->c[cB] = t;
      break;
    case 3:
      out->c[cR] = p;
      out->c[cG] = q;
      out->c[cB] = in->c[cV];
      break;
    case 4:
      out->c[cR] = t;
      out->c[cG] = p;
      out->c[cB] = in->c[cV];
      break;
    case 5:
      out->c[cR] = in->c[cV];
      out->c[cG] = p;
      out->c[cB] = q;
      break;
    }
  }
  out->c[cA] = in->c[cA];
}

void colour_f_yuv2hsv(const colour_floats *in, colour_floats *out) {
  colour_floats rgb;
  colour_f_yuv2rgb(in, &rgb);
  colour_f_rgb2hsv(&rgb, out);
}

void colour_f_hsv2yuv(const colour_floats *in, colour_floats *out) {
  colour_floats rgb;
  colour_f_hsv2rgb(in, &rgb);
  colour_f_rgb2yuv(&rgb, out);
}

/* uint8_t */

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

void colour_b_rgb2hsv(const colour_bytes *in, colour_bytes *out) {
  colour_floats inf, outf;
  colour_b2f(in, &inf);
  colour_f_rgb2hsv(&inf, &outf);
  colour_f2b(&outf, out);
}

void colour_b_hsv2rgb(const colour_bytes *in, colour_bytes *out) {
  colour_floats inf, outf;
  colour_b2f(in, &inf);
  colour_f_hsv2rgb(&inf, &outf);
  colour_f2b(&outf, out);
}

void colour_b_yuv2hsv(const colour_bytes *in, colour_bytes *out) {
  colour_floats inf, outf;
  colour_b2f(in, &inf);
  colour_f_yuv2hsv(&inf, &outf);
  colour_f2b(&outf, out);
}

void colour_b_hsv2yuv(const colour_bytes *in, colour_bytes *out) {
  colour_floats inf, outf;
  colour_b2f(in, &inf);
  colour_f_hsv2yuv(&inf, &outf);
  colour_f2b(&outf, out);
}

/* Arrays */

#define ARRAY_OP(sig, type) \
  void colour__PASTE(sig, _array)(const type *in, type *out, size_t len) { \
    for (unsigned i = 0; i < len; i++)                              \
      sig(in++, out++);                                             \
  }

#define ARRAY_OPS(op) \
  ARRAY_OP(colour__PASTE(colour_f_, op), colour_floats) \
  ARRAY_OP(colour__PASTE(colour_b_, op), colour_bytes)

#define X ARRAY_OPS
colour_OPS
#undef X

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
