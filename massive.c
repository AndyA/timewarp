/* massive.c */

#include <stdint.h>
#include <string.h>
#include <math.h>

#include <jd_pretty.h>

#include "average.h"
#include "filter.h"
#include "massive.h"
#include "model.h"
#include "yuv4mpeg2.h"

#define SHIFT 23
#define SIGMA 0.00001

typedef struct {
  double position;
  double velocity;
  double average;
  double rms;
} massive__datum;

typedef struct {
  int disabled;
  double mass;
  double drag;
  double attraction;
  double mix;
  double rms_fwd_weight;
  double rms_inv_weight;
  double intensity_fwd_mass;
  double intensity_inv_mass;
  average average;
  average rms_acc;
} massive__plane;

typedef struct {
  massive__datum *data;
  y4m2_frame *prev;
} massive__work;

static void massive__free(massive__work *wrk) {
  if (wrk) {
    jd_free(wrk->data);
    y4m2_release_frame(wrk->prev);
    jd_free(wrk);
  }
}

static void massive__setup(y4m2_frame *frame, massive__work *wrk) {
  massive__datum *dp = wrk->data = jd_alloc(frame->i.size * sizeof(massive__datum));
  uint8_t *fp = frame->buf;

  for (unsigned p = 0; p < Y4M2_N_PLANE; p++) {
    for (unsigned i = 0; i < frame->i.plane[p].size; i++) {
      dp->position = dp->rms = *fp++;
      dp->velocity = dp->average = 0;
    }
  }
}

static const char *plane_key[] = { "Y", "Cb", "Cr" };

static void massive__config_plane(massive__plane *p, jd_var *opt) {
  p->disabled = model_get_int(opt, 0, "$.disabled");
  p->mass = model_get_real(opt, 1, "$.mass");
  p->drag = model_get_real(opt, 0, "$.drag");
  p->attraction = model_get_real(opt, 1, "$.attraction");
  p->mix = model_get_real(opt, 1, "$.mix");

  p->rms_fwd_weight = model_get_real(opt, 0, "$.rms_fwd_weight");
  p->rms_inv_weight = model_get_real(opt, 1, "$.rms_inv_weight");

  p->intensity_fwd_mass = model_get_real(opt, 0, "$.intensity_fwd_mass");
  p->intensity_inv_mass = model_get_real(opt, 0, "$.intensity_inv_mass");

  average_config(&p->average, opt, "rms");
  average_config(&p->rms_acc, opt, "rms");
}

static void massive__config_parse(massive__plane *pl, jd_var *opt) {
  for (unsigned p = 0; p < Y4M2_N_PLANE; p++) {
    jd_var *slot = jd_rv(opt, "$.options.%s", plane_key[p]);
    massive__config_plane(pl++, slot);
  }
}

static void massive__start(filter *filt, const y4m2_parameters *parms) {
  if (!filt->ctx) filt->ctx = jd_alloc(sizeof(massive__work));
  y4m2_emit_start(filt->out, parms);
}

static void massive__frame(filter *filt, const y4m2_parameters *parms, y4m2_frame *frame) {
  massive__work *wrk = filt->ctx;
  massive__plane plane[Y4M2_N_PLANE];
  if (!wrk->data) massive__setup(frame, wrk);
  massive__config_parse(plane, &filt->config);

  if (wrk->prev) {
    massive__datum *dp = wrk->data;
    uint8_t *fp = frame->buf;
    uint8_t *pp = wrk->prev->buf;

    for (unsigned p = 0; p < Y4M2_N_PLANE; p++) {
      if (plane[p].disabled) {
        dp += frame->i.plane[p].size;
        fp += frame->i.plane[p].size;
        pp += frame->i.plane[p].size;
      }
      else {
        double min = (p == Y4M2_Y_PLANE) ? 16 : 16;
        double max = (p == Y4M2_Y_PLANE) ? 235 : 240;
        double drag = plane[p].drag;
        double mass = plane[p].mass;
        double attraction = plane[p].attraction;
        double mix = plane[p].mix;
        double fwd_weight = plane[p].rms_fwd_weight;
        double inv_weight = plane[p].rms_inv_weight;
        double fwd_mass = plane[p].intensity_fwd_mass;
        double inv_mass = plane[p].intensity_inv_mass;

        int do_rms = fwd_weight != 0 || inv_weight != 1;

        for (unsigned i = 0; i < frame->i.plane[p].size; i++) {
          double local_mass = mass +
                              (*fp * fwd_mass) +
                              ((255 - *fp) * inv_mass);

          if (do_rms) {
            dp->average = average_next(&plane[p].average, dp->average, *fp);
            double dy = *fp - dp->average;
            double rms = sqrt(dp->rms = average_next(
                                          &plane[p].rms_acc, dp->rms, dy * dy));
            if (rms < SIGMA) rms = SIGMA;
            local_mass += (fwd_weight * rms) + (inv_weight / rms);
          }

          if (local_mass < SIGMA) local_mass = SIGMA;

          double force = (*fp - dp->position) * attraction -
                         (dp->velocity * fabs(dp->velocity) * drag);
          dp->velocity += force / local_mass;
          double sample = dp->position += dp->velocity;
          if (sample < min) sample = min;
          if (sample > max) sample = max;
          *pp++ = mix * (uint8_t) sample + (1 - mix) * *fp++;
          dp++;
        }

        if (do_rms) {
          average_update(&plane[p].average);
          average_update(&plane[p].rms_acc);
        }
      }
    }

    wrk->prev->sequence = frame->sequence;
    y4m2_emit_frame(filt->out, parms, wrk->prev);
    y4m2_release_frame(wrk->prev);
  }
  else {
    y4m2_emit_frame(filt->out, parms, frame);
  }

  wrk->prev = y4m2_retain_frame(frame);
}

static void massive__end(filter *filt) {
  y4m2_emit_end(filt->out);
  massive__free(filt->ctx);
}

void massive_register(void) {
  filter f = {
    .start = massive__start,
    .frame = massive__frame,
    .end = massive__end
  };
  filter_register("massive", &f);
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
