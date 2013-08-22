/* colour.c */

#include <math.h>
#include <stdlib.h>

#include "framework.h"
#include "tap.h"
#include "colour.h"

#define SIGMA_F 0.1
#define SIGMA_B 7

#define countof(x) (sizeof(x) / sizeof((x)[0]))

typedef void (*fmap)(const colour_floats *in, colour_floats *out);
typedef void (*bmap)(const colour_bytes *in, colour_bytes *out);

static void roundtrip_floats(const char *title,
                             const colour_floats *in, size_t count,
                             fmap fwd, fmap inv) {

  for (unsigned i = 0; i < count; i++) {
    const colour_floats *ip = in + i;
    colour_floats tmp, out;
    char desc[200];

    snprintf(desc, sizeof(desc), "[%g, %g, %g, %g]",
             ip->c[0], ip->c[1], ip->c[2], ip->c[3]);

    fwd(ip, &tmp);
    inv(&tmp, &out);
    for (int c = 0; c < cMAX; c++) {
      ok(fabs(ip->c[c] - out.c[c]) <= SIGMA_F,
         "%s %s: round trip %d %g -> %g -> %g",
         title, desc, c, ip->c[c], tmp.c[c], out.c[c]);
    }
  }
}

static void roundtrip_bytes(const char *title,
                            const colour_bytes *in, size_t count,
                            bmap fwd, bmap inv) {

  for (unsigned i = 0; i < count; i++) {
    const colour_bytes *ip = in + i;
    colour_bytes tmp, out;
    char desc[200];

    snprintf(desc, sizeof(desc), "[%d, %d, %d, %d]",
             ip->c[0], ip->c[1], ip->c[2], ip->c[3]);

    fwd(ip, &tmp);
    inv(&tmp, &out);
    for (int c = 0; c < cMAX; c++) {
      ok(abs(ip->c[c] - out.c[c]) <= SIGMA_B,
         "%s %s: round trip %d %d -> %d -> %d",
         title, desc, c, ip->c[c], tmp.c[c], out.c[c]);
    }
  }
}

static void test_floats_rgb2yuv(void) {
  static colour_floats rgb2yuv[] = {
    {{ 0, 0, 0, 0 }},
    {{ 128, 128, 128, 0 }},
    {{ 255, 255, 255, 0 }},
    {{ 0, 128, 255, 0 }},
    {{ 255, 0, 128, 0 }},
    {{ 128, 255, 0, 0 }}
  };

  roundtrip_floats("rgb2yuv, yuv2rgb, float",
                   rgb2yuv, countof(rgb2yuv),
                   colour_f_rgb2yuv, colour_f_yuv2rgb);
}

static void test_floats_yuv2rgb(void) {
  static colour_floats yuv2rgb[] = {
    {{ 16, 16, 16, 0 }},
    {{ 128, 128, 128, 0 }},
    {{ 235, 240, 240, 0 }},
    {{ 16, 128, 240, 0 }},
    {{ 235, 16, 128, 0 }},
    {{ 128, 240, 16, 0 }}
  };

  roundtrip_floats("yuv2rgb, rgb2yuv, float",
                   yuv2rgb, countof(yuv2rgb),
                   colour_f_yuv2rgb, colour_f_rgb2yuv);
}

static void test_floats_rgb2hsv(void) {
  static colour_floats rgb2hsv[] = {
    {{ 0, 0, 0, 0 }},
    {{ 128, 128, 128, 0 }},
    {{ 255, 255, 255, 0 }},
    {{ 0, 128, 255, 0 }},
    {{ 255, 0, 128, 0 }},
    {{ 128, 255, 0, 0 }}
  };

  roundtrip_floats("rgb2hsv, hsv2rgb, float",
                   rgb2hsv, countof(rgb2hsv),
                   colour_f_rgb2hsv, colour_f_hsv2rgb);
}

static void test_floats_hsv2rgb(void) {
  static colour_floats hsv2rgb[] = {
    {{ 20, 20, 20, 20 }},
    {{ 128, 128, 128, 20 }},
    {{ 235, 235, 235, 20 }},
    {{ 20, 128, 235, 20 }},
    {{ 235, 20, 128, 20 }},
    {{ 128, 235, 20, 20 }}
  };

  roundtrip_floats("hsv2rgb, rgb2hsv, float",
                   hsv2rgb, countof(hsv2rgb),
                   colour_f_hsv2rgb, colour_f_rgb2hsv);
}



static void test_bytes_rgb2yuv(void) {
  static colour_bytes rgb2yuv[] = {
    {{ 0, 0, 0, 0 }},
    {{ 128, 128, 128, 0 }},
    {{ 255, 255, 255, 0 }},
    {{ 0, 128, 255, 0 }},
    {{ 255, 0, 128, 0 }},
    {{ 128, 255, 0, 0 }}
  };

  roundtrip_bytes("rgb2yuv, yuv2rgb, byte",
                  rgb2yuv, countof(rgb2yuv),
                  colour_b_rgb2yuv, colour_b_yuv2rgb);
}

static void test_bytes_yuv2rgb(void) {
  static colour_bytes yuv2rgb[] = {
    {{ 16, 128, 128, 0 }},
    {{ 128, 128, 128, 0 }},
    {{ 235, 128, 128, 0 }},
    {{ 16, 150, 128, 0 }},
    {{ 128, 130, 128, 0 }},
    {{ 235, 120, 128, 0 }},
  };

  roundtrip_bytes("yuv2rgb, rgb2yuv, byte",
                  yuv2rgb, countof(yuv2rgb),
                  colour_b_yuv2rgb, colour_b_rgb2yuv);
}

static void test_bytes_rgb2hsv(void) {
  static colour_bytes rgb2hsv[] = {
    {{ 0, 0, 0, 0 }},
    {{ 128, 128, 128, 0 }},
    {{ 255, 255, 255, 0 }},
    {{ 0, 128, 255, 0 }},
    {{ 255, 0, 128, 0 }},
    {{ 128, 255, 0, 0 }}
  };

  roundtrip_bytes("rgb2hsv, hsv2rgb, byte",
                  rgb2hsv, countof(rgb2hsv),
                  colour_b_rgb2hsv, colour_b_hsv2rgb);
}

static void test_bytes_hsv2rgb(void) {
  static colour_bytes hsv2rgb[] = {
    {{ 20, 20, 20, 20 }},
    {{ 128, 128, 128, 20 }},
    {{ 235, 235, 235, 20 }},
    {{ 20, 128, 235, 20 }},
    {{ 235, 20, 128, 20 }},
    {{ 128, 235, 20, 20 }}
  };

  roundtrip_bytes("hsv2rgb, rgb2hsv, byte",
                  hsv2rgb, countof(hsv2rgb),
                  colour_b_hsv2rgb, colour_b_rgb2hsv);
}


void test_main(void) {
  test_floats_rgb2yuv();
  test_floats_yuv2rgb();
  test_floats_rgb2hsv();
  test_floats_hsv2rgb();

  test_bytes_rgb2yuv();
  test_bytes_yuv2rgb();
  test_bytes_rgb2hsv();
  test_bytes_hsv2rgb();
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
