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

static filter *filter__clone(filter *f) {
  filter *nf = jd_alloc(sizeof(filter));
  *nf = *f;
  jd_assign(&nf->config, &f->config);
  return nf;
}

void filter_register(const char *name, filter *f) {
  jd_set_object(jd_get_ks(&filters, name, 1), filter__clone(f), jd_free);
}

static void filter__free(filter *filt) {
  jd_release(&filt->config);
  jd_release(&filt->model);
  jd_free(filt);
}

static void filter__configure(filter *filt, jd_var *opt) {
  jd_assign(&filt->config, opt);
}

static void filter__update_config(filter *filt) {
  jd_var *config = model_multi_load(jd_nv(), &filt->model);
  filter__configure(filt, jd_get_idx(config, filt->idx));
}

static void filter__callback(y4m2_reason reason,
                             const y4m2_parameters *parms,
                             y4m2_frame *frame,
                             void *ctx) {
  filter *filt = (filter *) ctx;
  switch (reason) {
  case Y4M2_START:
    if (filt->start)
      filt->start(filt, parms);
    else
      y4m2_emit_start(filt->out, parms);
    break;
  case Y4M2_FRAME:
    filter__update_config(filt);
    if (!filt->frame || model_get_int(&filt->config, 0, "$.disabled"))
      y4m2_emit_frame(filt->out, parms, frame);
    else
      filt->frame(filt, parms, frame);
    break;
  case Y4M2_END:
    if (filt->end)
      filt->end(filt);
    else
      y4m2_emit_end(filt->out);
    filter__free(filt);
    break;
  }
}

static y4m2_output *filter__hook(const char *name,
                                 y4m2_output *out,
                                 jd_var *model,
                                 unsigned idx) {
  jd_var *fp = jd_get_ks(&filters, name, 0);
  if (!fp) jd_throw("Unknown filter \"%s\"", name);

  jd_var *config = model_multi_load(jd_nv(), model);
  jd_var *opt = jd_get_idx(config, idx);

  filter *filt = filter__clone(jd_ptr(fp));
  filt->ctx = NULL;
  filt->out = out;
  jd_assign(&filt->model, model);
  filt->idx = idx;
  filter__configure(filt, opt);
  return y4m2_output_next(filter__callback, filt);
}

y4m2_output *filter_build(y4m2_output *out, jd_var *model) {
  y4m2_output *prev = out;
  scope {
    jd_var *config = model_multi_load(jd_nv(), model);
    for (int i = jd_count(config); --i >= 0;) {
      jd_var *opt = jd_get_idx(config, i);
      prev = filter__hook(jd_bytes(jd_rv(opt, "$.filter"), NULL), prev, model, i);
    }
  }
  return prev;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
