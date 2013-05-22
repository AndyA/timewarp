/* average.c */

#include <jd_pretty.h>

#include "average.h"

void average_init(average *avg, double decay, double ow, double nw) {
  avg->decay = decay;
  avg->ow = ow;
  avg->nw = nw;

  avg->scale = 1;
  avg->tw = ow + nw;
}

static double get_real(jd_var *v, double fallback) {
  return v ? jd_get_real(v) : fallback;
}

void average_config(average *avg, jd_var *v, const char *ns) {
  average_init(avg,
               get_real(jd_lv(v, "$.%s_decay", ns), 0),
               get_real(jd_lv(v, "$.%s_ow", ns), 1),
               get_real(jd_lv(v, "$.%s_nw", ns), 1));
}

void average_update(average *avg) {
  avg->scale = (avg->scale * avg->decay * avg->ow) + avg->nw;
}

double average_next(average *avg, double prev, double next) {
  return (prev * avg->decay * avg->ow + next * avg->nw) / (avg->scale * avg->tw);
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
