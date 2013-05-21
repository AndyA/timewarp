/* yuv4mpeg2.c */

#include <string.h>

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

void test_main(void) {
  test_parms();
  test_parse();
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
