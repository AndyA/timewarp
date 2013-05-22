/* massive.c */

#include <stdint.h>
#include <string.h>
#include <math.h>

#include <jd_pretty.h>

#include "yuv4mpeg2.h"
#include "average.h"
#include "massive.h"

#define SHIFT 23

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
    double rms_weight;
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
      dp->position = *fp++;
      dp->velocity = 0;
      dp->average = 0;
      dp->rms = 0;
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
        int do_rms = wrk->plane[p].rms_weight != 0;
        for (unsigned i = 0; i < frame->i.plane[p].size; i++) {
          double local_mass = mass;
          if (do_rms) {
            dp->average = average_next(&(wrk->plane[p].average), dp->average, *fp);
            double dy = *fp - dp->average;
            dp->rms = average_next(&(wrk->plane[p].rms_acc), dp->rms, dy * dy);
            local_mass = mass; // + wrk->plane[p].rms_weight * sqrt(dp->rms);
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
    wrk->plane[p].disabled = jd_get_int(jd_lv(opt, "$.%s.disabled", plane_key[p]));
    wrk->plane[p].mass = jd_get_real(jd_lv(opt, "$.%s.mass", plane_key[p]));
    wrk->plane[p].drag = jd_get_real(jd_lv(opt, "$.%s.drag", plane_key[p]));

    wrk->plane[p].rms_weight = jd_get_real(jd_lv(opt, "$.%s.rms_weight", plane_key[p]));
    average_config(&(wrk->plane[p].average), opt, "rms_");
    average_config(&(wrk->plane[p].rms_acc), opt, "rms_");
  }
  return y4m2_output_next(massive__callback, wrk);
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
