/* hist.c */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "hist.h"

static void hist__interp(double *map, size_t size, double lo, double hi) {
  if (map[0] < 0) map[0] = lo;
  if (map[size - 1] < 0) map[size - 1] = hi;
  for (unsigned pos = 0; pos < size - 1;) {
    unsigned opos = pos;
    double start = map[pos++];
    while (pos < size - 1 && map[pos] < 0) pos++;
    double inc = (map[pos] - start) / (pos - opos);
    for (unsigned i = opos + 1; i < pos; i++)
      map[i] = (start += inc);
  }
}

static void hist__clear(double *map, size_t size) {
  for (unsigned i = 0; i < size; i++) map[i] = -1;
}

static void hist__clone(const double *in, double *out, size_t size) {
  memcpy(out, in, sizeof(double) * size);
}

static void hist__cum(const uint32_t *freq, size_t size, double *cum) {
  double next = 0;
  for (unsigned i = 0; i < size; i++) {
    cum[i] = next;
    next += freq[i];
  }
}

static int hist__half(const double *map, int lo, int hi) {
  double half = (map[lo] + map[hi - 1]) / 2;
  while (lo < hi) {
    int mid = (lo + hi) / 2;
    if (map[mid] < half) lo = mid + 1;
    else hi = mid - 1;
  }
  return lo;
}

static void hist__build_map(double *hmap, int hlo, int hhi,
                            const double *cum, int clo, int chi) {
  if (hlo >= hhi || clo >= chi) return;
  int half = hist__half(cum, clo, chi);
  int mid = (hlo + hhi) / 2;
  hmap[mid] = half;
  hist__build_map(hmap, hlo, mid - 1, cum, clo, half - 1);
  hist__build_map(hmap, mid + 1, hhi, cum, half + 1, chi);
}

void hist_map(const uint32_t *freq, size_t size, double *map) {
  double cum[size];
  hist__cum(freq, size, cum);
  hist__clear(map, size);
  hist__build_map(map, 0, size - 1, cum, 0, size - 1);
  hist__interp(map, size, 0, size - 1);
}

void hist_inverse(const double *in, double *out, size_t size) {
  double tmp[size];

  hist__clear(out, size);
  hist__clone(in, tmp, size);
  hist__interp(tmp, size, 0, size - 1);

  for (unsigned i = 0; i < size; i++)
    out[(unsigned) tmp[i]] = i;

  hist__interp(out, size, 0, size - 1);
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
