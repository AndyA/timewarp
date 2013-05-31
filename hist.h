/* hist.h */

#ifndef HIST_H_
#define HIST_H_

void hist_map(const uint32_t *freq, size_t size, double *map);
void hist_inverse(const double *in, double *out, size_t size);

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
