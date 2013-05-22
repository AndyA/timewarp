/* average.c */

#include <jd_pretty.h>

#include "average.h"
#include "util.h"

void average_init(average *avg, double decay, double ow, double nw) {
  avg->decay = decay;
  avg->ow = ow;
  avg->nw = nw;

  avg->scale = 1;
  avg->tw = ow + nw;
}

void average_config(average *avg, jd_var *v, const char *ns) {
  average_init(avg,
               util_get_real(jd_rv(v, "$.%s_decay", ns), 0),
               util_get_real(jd_rv(v, "$.%s_ow", ns), 1),
               util_get_real(jd_rv(v, "$.%s_nw", ns), 1));
}

void average_update(average *avg) {
  avg->scale = (avg->scale * avg->decay * avg->ow) + avg->nw;
}

double average_next(average *avg, double prev, double next) {
  return (prev * avg->decay * avg->ow + next * avg->nw) / (avg->scale * avg->tw);
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
