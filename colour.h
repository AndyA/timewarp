/* colour.h */

#ifndef COLOUR_H_
#define COLOUR_H_

#include <stdint.h>
#include <stdlib.h>

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

/* doubles */

void colour_f_rgb2yuv(const colour_floats *in, colour_floats *out);
void colour_f_yuv2rgb(const colour_floats *in, colour_floats *out);

void colour_f_rgb2hsv(const colour_floats *in, colour_floats *out);
void colour_f_hsv2rgb(const colour_floats *in, colour_floats *out);

void colour_f_yuv2hsv(const colour_floats *in, colour_floats *out);
void colour_f_hsv2yuv(const colour_floats *in, colour_floats *out);

void colour_f_rgb2yuv_array(const colour_floats *in, colour_floats *out, size_t len);
void colour_f_yuv2rgb_array(const colour_floats *in, colour_floats *out, size_t len);

void colour_f_rgb2hsv_array(const colour_floats *in, colour_floats *out, size_t len);
void colour_f_hsv2rgb_array(const colour_floats *in, colour_floats *out, size_t len);

void colour_f_yuv2hsv_array(const colour_floats *in, colour_floats *out, size_t len);
void colour_f_hsv2yuv_array(const colour_floats *in, colour_floats *out, size_t len);

/* uint8_t */

void colour_b_rgb2yuv(const colour_bytes *in, colour_bytes *out);
void colour_b_yuv2rgb(const colour_bytes *in, colour_bytes *out);

void colour_b_rgb2hsv(const colour_bytes *in, colour_bytes *out);
void colour_b_hsv2rgb(const colour_bytes *in, colour_bytes *out);

void colour_b_yuv2hsv(const colour_bytes *in, colour_bytes *out);
void colour_b_hsv2yuv(const colour_bytes *in, colour_bytes *out);

void colour_b_rgb2yuv_array(const colour_bytes *in, colour_bytes *out, size_t len);
void colour_b_yuv2rgb_array(const colour_bytes *in, colour_bytes *out, size_t len);

void colour_b_rgb2hsv_array(const colour_bytes *in, colour_bytes *out, size_t len);
void colour_b_hsv2rgb_array(const colour_bytes *in, colour_bytes *out, size_t len);

void colour_b_yuv2hsv_array(const colour_bytes *in, colour_bytes *out, size_t len);
void colour_b_hsv2yuv_array(const colour_bytes *in, colour_bytes *out, size_t len);

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
