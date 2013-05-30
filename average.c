/* average.c */

#include <jd_pretty.h>

#include "average.h"
#include "model.h"

void average_init(average *avg, double decay, double ow, double nw) {
  avg->decay = decay;
  avg->ow = ow;
  avg->nw = nw;

  avg->scale = 1;
  avg->tw = ow + nw;
}

void average_config(average *avg, jd_var *v, const char *ns) {
  average_init(avg,
               model_get_real(v, 0, "$.%s_decay", ns),
               model_get_real(v, 1, "$.%s_ow", ns),
               model_get_real(v, 1, "$.%s_nw", ns));
}

void average_update(average *avg) {
  avg->scale = (avg->scale * avg->decay * avg->ow) + avg->nw;
}

double average_next(average *avg, double prev, double next) {
  return (prev * avg->decay * avg->ow + next * avg->nw) / (avg->scale * avg->tw);
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
