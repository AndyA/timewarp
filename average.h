/* average.h */

#ifndef AVERAGE_H_
#define AVERAGE_H_

typedef struct {
  double decay;
  double ow;
  double nw;

  double scale;
  double tw;
} average;

void average_init(average *avg, double decay, double ow, double nw);
void average_config(average *avg, jd_var *v, const char *ns);
void average_update(average *avg);
double average_next(average *avg, double prev, double next);

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
