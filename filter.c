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
  jd_free(filt);
}

static void filter__free_cb(void *f) {
  filter__free(f);
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
    break;
  }
}

static y4m2_output *filter__hook(jd_var *stash, const char *name, y4m2_output *out) {
  (void) stash;
  jd_var *fp = jd_get_ks(&filters, name, 0);
  if (!fp) jd_throw("Unknown filter \"%s\"", name);

  filter *filt = filter__clone(jd_ptr(fp));
  filt->ctx = NULL;
  filt->out = out;
  jd_set_object(jd_unshift(stash, 1), filt, filter__free_cb);
  return y4m2_output_next(filter__callback, filt);
}

static void filter__update_config(jd_var *stash) {
  scope {
    jd_var *config = model_multi_load(jd_nv(), jd_get_ks(stash, "model", 0));
    jd_var *list = jd_get_ks(stash, "list", 0);
    size_t count = jd_count(list);
    for (unsigned i = 0; i < count; i++) {
      filter *filt = jd_ptr(jd_get_idx(list, i));
      jd_assign(&filt->config, jd_get_idx(config, i));
    }
  }
}

typedef struct {
  y4m2_output *out;
  jd_var stash;
} filter__control;

static void filter__control_free(filter__control *fc) {
  jd_release(&fc->stash);
  jd_free(fc);
}

static void filter__control_cb(y4m2_reason reason,
                               const y4m2_parameters *parms,
                               y4m2_frame *frame,
                               void *ctx) {
  filter__control *fc = (filter__control *) ctx;
  switch (reason) {
  case Y4M2_START:
    y4m2_emit_start(fc->out, parms);
    break;
  case Y4M2_FRAME:
    filter__update_config(&fc->stash);
    y4m2_emit_frame(fc->out, parms, frame);
    break;
  case Y4M2_END:
    y4m2_emit_end(fc->out);
    filter__control_free(fc);
    break;
  }
}

static y4m2_output *filter__control_hook(jd_var *stash, y4m2_output *out) {
  filter__control *fc = jd_alloc(sizeof(filter__control));
  fc->out = out;
  jd_assign(&fc->stash, stash);
  return y4m2_output_next(filter__control_cb, fc);
}

y4m2_output *filter_build(jd_var *stash, y4m2_output *out, jd_var *model) {
  y4m2_output *prev = out;
  scope {
    jd_set_hash(stash, 10);
    jd_assign(jd_get_ks(stash, "model", 1), model);
    jd_var *config = model_multi_load(jd_nv(), model);
    size_t nfilt = jd_count(config);
    jd_var *list = jd_set_array(jd_get_ks(stash, "list", 1), nfilt);
    for (int i = (int) nfilt; --i >= 0;) {
      jd_var *opt = jd_get_idx(config, i);
      const char *name = jd_bytes(jd_rv(opt, "$.filter"), NULL);
      prev = filter__hook(list, name, prev);
    }
  }
  return filter__control_hook(stash, prev);
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
