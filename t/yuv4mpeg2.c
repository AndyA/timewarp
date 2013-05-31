/* yuv4mpeg2.c */

#include <stdlib.h>
#include <string.h>

#include "colour.h"
#include "framework.h"
#include "tap.h"
#include "yuv4mpeg2.h"

static void test_parms(void) {
  y4m2_parameters *p = y4m2_new_parms();

  ok(p != NULL, "y4m2_new_parms");

  y4m2_set_parm(p, "W", "1920");
  y4m2_set_parm(p, "H", "1080");

  ok(!strcmp("1920", y4m2_get_parm(p, "W")), "get W");
  ok(!strcmp("1080", y4m2_get_parm(p, "H")), "get H");
  ok(NULL == y4m2_get_parm(p, "a"), "out of range");
  ok(NULL == y4m2_get_parm(p, "AAAA"), "too long");
  ok(NULL == y4m2_get_parm(p, "Z"), "missing");

  y4m2_parameters *p2 = y4m2_clone_parms(p);

  ok(!strcmp("1920", y4m2_get_parm(p, "W")), "get W");
  ok(!strcmp("1080", y4m2_get_parm(p, "H")), "get H");
  ok(!strcmp("1920", y4m2_get_parm(p2, "W")), "get W");
  ok(!strcmp("1080", y4m2_get_parm(p2, "H")), "get H");

  y4m2_set_parm(p, "W", "1280");
  y4m2_set_parm(p2, "H", "1024");

  ok(!strcmp("1280", y4m2_get_parm(p, "W")), "get W");
  ok(!strcmp("1080", y4m2_get_parm(p, "H")), "get H");
  ok(!strcmp("1920", y4m2_get_parm(p2, "W")), "get W");
  ok(!strcmp("1024", y4m2_get_parm(p2, "H")), "get H");

  y4m2_free_parms(p);
  y4m2_free_parms(p2);
}

static void test_parse(void) {
  y4m2_parameters *p = y4m2_new_parms();

  char pstr[] = "W1920 H1080 A1:1 Ip F25:1 C420\n";

  y4m2__parse_parms(p, pstr);

  ok(!strcmp("1920", y4m2_get_parm(p, "W")), "get W");
  ok(!strcmp("1080", y4m2_get_parm(p, "H")), "get H");
  ok(!strcmp("1:1", y4m2_get_parm(p, "A")), "get A");
  ok(!strcmp("p", y4m2_get_parm(p, "I")), "get I");
  ok(!strcmp("25:1", y4m2_get_parm(p, "F")), "get F");
  ok(!strcmp("420", y4m2_get_parm(p, "C")), "get C");

  y4m2_frame_info info;
  y4m2_parse_frame_info(&info, p);
  ok(info.size == 1920 * 1080 * 3 / 2, "frame size");

  y4m2_free_parms(p);
}

static void random_frame(y4m2_frame *frame) {
  uint8_t *bp = frame->buf;
  for (unsigned p = 0; p < Y4M2_N_PLANE; p++) {
    unsigned min = 16;
    unsigned max = (p == Y4M2_Y_PLANE ? 235 : 240);
    for (unsigned i = 0; i < frame->i.plane[p].size; i++) {
      uint8_t sample;
      do {
        sample = rand() & 0xFF;
      }
      while (sample < min || sample > max);
      *bp++ = sample;
    }
  }
}

static void test_float(void) {
  y4m2_parameters *p = y4m2_new_parms();
  char pstr[] = "W1280 H720 A1:1 Ip F25:1 C420\n";
  y4m2__parse_parms(p, pstr);

  y4m2_frame *frame = y4m2_new_frame(p);

  size_t fsize = frame->i.width * frame->i.height;
  colour_floats *ff = jd_alloc(sizeof(colour_floats) * fsize);

  random_frame(frame);
  y4m2_frame *tmp = y4m2_like_frame(frame);

  y4m2_frame_to_float(frame, ff);
  y4m2_float_to_frame(ff, tmp);

  ok(memcmp(tmp->buf, frame->buf, frame->i.size) == 0,
     "frame_to_float, float_to_frame");

  jd_free(ff);
  y4m2_release_frame(tmp);
  y4m2_release_frame(frame);
  y4m2_free_parms(p);
}

void test_main(void) {
  test_parms();
  test_parse();
  test_float();
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
