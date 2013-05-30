/* massive.c */

#include <stdint.h>
#include <string.h>
#include <math.h>

#include <jd_pretty.h>

#include "yuv4mpeg2.h"
#include "average.h"
#include "massive.h"
#include "util.h"

#define SHIFT 23
#define SIGMA 0.00001

typedef struct {
  double position;
  double velocity;
  double average;
  double rms;
} massive__datum;

typedef struct {
  y4m2_output *out;

  struct {
    int disabled;
    double mass;
    double drag;
    double rms_fwd_weight;
    double rms_inv_weight;
    double intensity_fwd_mass;
    double intensity_inv_mass;
    average average;
    average rms_acc;
  } plane[Y4M2_N_PLANE];

  massive__datum *data;

  y4m2_frame *prev;

} massive__work;

static void massive__free(massive__work *wrk) {
  if (wrk) {
    y4m2_free(wrk->data);
    y4m2_release_frame(wrk->prev);
    y4m2_free(wrk);
  }
}

static void massive__setup(y4m2_frame *frame,
                           massive__work *wrk) {
  massive__datum *dp = wrk->data = y4m2_alloc(frame->i.size * sizeof(massive__datum));

  uint8_t *fp = frame->buf;

  for (unsigned p = 0; p < Y4M2_N_PLANE; p++) {
    for (unsigned i = 0; i < frame->i.plane[p].size; i++) {
      dp->position = dp->rms = *fp++;
      dp->velocity = dp->average = 0;
    }
  }
}

static void massive__frame(y4m2_frame *frame,
                           const y4m2_parameters *parms,
                           massive__work *wrk) {
  if (!wrk->data) massive__setup(frame, wrk);

  if (wrk->prev) {
    massive__datum *dp = wrk->data;
    uint8_t *fp = frame->buf;
    uint8_t *pp = wrk->prev->buf;

    for (unsigned p = 0; p < Y4M2_N_PLANE; p++) {
      if (!wrk->plane[p].disabled) {
        double min = (p == Y4M2_Y_PLANE) ? 16 : 0;
        double max = (p == Y4M2_Y_PLANE) ? 235 : 255;
        double drag = wrk->plane[p].drag;
        double mass = wrk->plane[p].mass;
        double fwd_weight = wrk->plane[p].rms_fwd_weight;
        double inv_weight = wrk->plane[p].rms_inv_weight;
        double fwd_mass = wrk->plane[p].intensity_fwd_mass;
        double inv_mass = wrk->plane[p].intensity_inv_mass;

        int do_rms = fwd_weight != 0 || inv_weight != 1;

        for (unsigned i = 0; i < frame->i.plane[p].size; i++) {
          double local_mass = mass +
                              (*fp * fwd_mass) +
                              ((255 - *fp) * inv_mass);

          if (do_rms) {
            dp->average = average_next(&wrk->plane[p].average, dp->average, *fp);
            double dy = *fp - dp->average;
            double rms = sqrt(dp->rms = average_next(
                                          &wrk->plane[p].rms_acc, dp->rms, dy * dy));
            if (rms < SIGMA) rms = SIGMA;
            local_mass = mass + (fwd_weight * rms) + (inv_weight / rms);
            if (local_mass < SIGMA) local_mass = SIGMA;
          }

          double force = (*fp - dp->position) - (dp->velocity * fabs(dp->velocity) * drag);
          dp->velocity += force / local_mass;

          double sample = dp->position += dp->velocity;
          if (sample < min) sample = min;
          if (sample > max) sample = max;
          *pp++ = (uint8_t) sample;
          fp++;
          dp++;
        }

        if (do_rms) {
          average_update(&wrk->plane[p].average);
          average_update(&wrk->plane[p].rms_acc);
        }
      }
    }

    y4m2_emit_frame(wrk->out, parms, wrk->prev);
    y4m2_release_frame(wrk->prev);
  }
  else {
    y4m2_emit_frame(wrk->out, parms, frame);
  }

  wrk->prev = y4m2_retain_frame(frame);
}

static void massive__callback(y4m2_reason reason,
                              const y4m2_parameters *parms,
                              y4m2_frame *frame,
                              void *ctx) {
  massive__work *wrk = (massive__work *) ctx;
  switch (reason) {
  case Y4M2_START:
    y4m2_emit_start(wrk->out, parms);
    break;
  case Y4M2_FRAME:
    massive__frame(frame, parms, wrk);
    break;
  case Y4M2_END:
    y4m2_emit_end(wrk->out);
    massive__free(wrk);
    break;
  }
}

static const char *plane_key[] = { "Y", "Cb", "Cr" };

y4m2_output *massive_hook(y4m2_output *out, jd_var *opt) {
  massive__work *wrk = y4m2_alloc(sizeof(massive__work));
  wrk->out = out;
  for (unsigned p = 0; p < Y4M2_N_PLANE; p++) {
    jd_var *slot = jd_rv(opt, "$.%s", plane_key[p]);

    wrk->plane[p].disabled = util_get_int(jd_rv(slot, "$.disabled"), 0);
    wrk->plane[p].mass = util_get_real(jd_rv(slot, "$.mass"), 1);
    wrk->plane[p].drag = util_get_real(jd_rv(slot, "$.drag"), 0);

    wrk->plane[p].rms_fwd_weight = util_get_real(jd_rv(slot, "$.rms_fwd_weight"), 0);
    wrk->plane[p].rms_inv_weight = util_get_real(jd_rv(slot, "$.rms_inv_weight"), 1);

    wrk->plane[p].intensity_fwd_mass = util_get_real(jd_rv(slot, "$.intensity_fwd_mass"), 0);
    wrk->plane[p].intensity_inv_mass = util_get_real(jd_rv(slot, "$.intensity_inv_mass"), 0);

    average_config(&wrk->plane[p].average, slot, "rms");
    average_config(&wrk->plane[p].rms_acc, slot, "rms");
  }
  return y4m2_output_next(massive__callback, wrk);
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
