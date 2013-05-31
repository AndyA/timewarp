/* colour.h */

#ifndef COLOUR_H_
#define COLOUR_H_

#include <stdint.h>
#include <stdlib.h>

#define colour__SPLICE(a, b)    a ## b
#define colour__PASTE(a, b)     colour__SPLICE(a, b)
#define colour__PASTE3(a, b, c) colour__PASTE(colour__PASTE(a, b), c)

enum { cR, cG, cB, cA, cMAX };
enum { cY, cCb, cCr };
enum { cH, cS, cV };

typedef struct {
  uint8_t c[cMAX];
} colour_bytes;

typedef struct {
  double c[cMAX];
} colour_floats;

void colour_f2b(const colour_floats *in, colour_bytes *out);
void colour_b2f(const colour_bytes *in, colour_floats *out);

#define colour_OPS \
  X(rgb2yuv) \
  X(yuv2rgb) \
  X(rgb2hsv) \
  X(hsv2rgb) \
  X(yuv2hsv) \
  X(hsv2yuv)

#define colour_DECLARE(op) \
  void colour__PASTE(colour_f_, op)(const colour_floats *in,          \
                                    colour_floats *out);              \
  void colour__PASTE3(colour_f_, op, _array)(const colour_floats *in, \
      colour_floats *out,      \
      size_t len);             \
  void colour__PASTE(colour_b_, op)(const colour_bytes *in,           \
                                    colour_bytes *out);               \
  void colour__PASTE3(colour_b_, op, _array)(const colour_bytes *in,  \
      colour_bytes *out,       \
      size_t len);

#define X colour_DECLARE
colour_OPS
#undef X

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
