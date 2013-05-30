/* filter.c */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <jd_pretty.h>

#include "filter.h"
#include "streak.h"
#include "stretch.h"
#include "model.h"
#include "yuv4mpeg2.h"

static jd_var filters = JD_INIT;

static void filter__teardown(void) {
  jd_release(&filters);
}

void filter_init(void) {
  jd_set_hash(&filters, 10);
  atexit(filter__teardown);
}

static filter *filter__clone(const filter *f) {
  filter *nf = jd_alloc(sizeof(filter));
  *nf = *f;
  return nf;
}

void filter_register(const char *name, filter *f) {
  jd_set_object(jd_get_ks(&filters, name, 1), filter__clone(f), jd_free);
}

static void filter__free(filter *filt) {
  jd_release(&filt->config);
  jd_free(filt);
}

static void filter__callback(y4m2_reason reason,
                             const y4m2_parameters *parms,
                             y4m2_frame *frame,
                             void *ctx) {
  filter *filt = (filter *) ctx;
  switch (reason) {
  case Y4M2_START:
    if (filt->start) filt->start(filt, parms);
    break;
  case Y4M2_FRAME:
    if (filt->frame) filt->frame(filt, parms, frame);
    break;
  case Y4M2_END:
    if (filt->end) filt->end(filt);
    filter__free(filt);
    break;
  }
}

static void filter__configure(filter *filt, jd_var *opt) {
  jd_assign(&filt->config, opt);
}

y4m2_output *filter_hook(const char *name, y4m2_output *out, jd_var *opt) {
  jd_var *fp = jd_get_ks(&filters, name, 0);
  if (!fp) jd_throw("Unknown filter \"%s\"", name);

  filter *f = filter__clone(jd_ptr(fp));
  f->ctx = NULL;
  f->out = out;
  filter__configure(f, opt);
  return y4m2_output_next(filter__callback, f);
}

y4m2_output *filter_build(y4m2_output *out, jd_var *config) {
  y4m2_output *last_out = out;

  for (int i = jd_count(config); --i >= 0;) {
    jd_var *filt = jd_get_idx(config, i);
    if (model_get_int(filt, 0, "$.disabled")) continue;
    last_out = filter_hook(jd_bytes(jd_rv(filt, "$.filter"), NULL),
                           last_out, jd_rv(filt, "$.options"));
  }

  return last_out;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
