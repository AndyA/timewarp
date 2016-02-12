/* ripple.c */

#include <math.h>
#include <stdint.h>
#include <string.h>

#include <jd_pretty.h>

#include "filter.h"
#include "model.h"
#include "ripple.h"
#include "yuv4mpeg2.h"

typedef struct {
  double *delay;
  size_t hist_size;
  y4m2_frame **history;
  y4m2_frame *buf;
  y4m2_parameters *parms;
} ripple__work;

typedef void (*ripple__generator)(filter *filt, const y4m2_frame *frame,
                                  double *delay);

static void ripple__resample(double *out, const double *in, unsigned width,
                             unsigned height, unsigned xs, unsigned ys) {
  if (xs == 1 && ys == 1) {
    memcpy(out, in, sizeof(double) * width * height);
    return;
  }

  unsigned ow = width / xs;
  unsigned oh = height / ys;
  double area = xs * ys;

  for (unsigned y = 0; y < oh; y++) {
    for (unsigned x = 0; x < ow; x++) {
      double total = 0;
      for (unsigned yy = 0; yy < ys; yy++) {
        for (unsigned xx = 0; xx < xs; xx++) {
          total += in[x * xs + xx + width * (y * ys + yy)];
        }
      }
      out[x + ow * y] = total / area;
    }
  }
}

static void ripple__pond(filter *filt, const y4m2_frame *frame, double *delay) {
  double frequency = model_get_real(&filt->config, 10, "$.options.frequency");

  double cx = (double)frame->i.width *
              model_get_real(&filt->config, 0.5, "$.options.centre_x");
  double cy = (double)frame->i.height *
              model_get_real(&filt->config, 0.5, "$.options.centre_y");

  for (unsigned y = 0; y < frame->i.height; y++) {
    for (unsigned x = 0; x < frame->i.width; x++) {
      double dx = x - cx;
      double dy = y - cy;
      delay[x + frame->i.width * y] =
          sin(sqrt(dx * dx + dy * dy) * 2 * M_PI / frequency);
    }
  }
}

static void ripple__radius(filter *filt, const y4m2_frame *frame,
                           double *delay) {

  double cx = (double)frame->i.width *
              model_get_real(&filt->config, 0.5, "$.options.centre_x");
  double cy = (double)frame->i.height *
              model_get_real(&filt->config, 0.5, "$.options.centre_y");

  for (unsigned y = 0; y < frame->i.height; y++) {
    for (unsigned x = 0; x < frame->i.width; x++) {
      double dx = x - cx;
      double dy = y - cy;
      delay[x + frame->i.width * y] = sqrt(dx * dx + dy * dy);
    }
  }
}

static void ripple__ramp(filter *filt, const y4m2_frame *frame, double *delay) {

  double angle =
      model_get_real(&filt->config, 0, "$.options.angle") * M_PI / 180.0;

  double sa = sin(angle);
  double ca = cos(angle);

  for (unsigned y = 0; y < frame->i.height; y++) {
    for (unsigned x = 0; x < frame->i.width; x++) {
      delay[x + frame->i.width * y] = x * ca - y * sa;
    }
  }
}

static struct {
  const char *name;
  ripple__generator gen;
} generators[] = {{.name = "pond", .gen = ripple__pond},
                  {.name = "radius", .gen = ripple__radius},
                  {.name = "ramp", .gen = ripple__ramp}};

static void ripple__free(ripple__work *wrk) {
  if (wrk) {
    jd_free(wrk->delay);
    for (unsigned i = 0; i < wrk->hist_size; i++)
      y4m2_release_frame(wrk->history[i]);
    jd_free(wrk->history);
    y4m2_release_frame(wrk->buf);
    y4m2_free_parms(wrk->parms);
    jd_free(wrk);
  }
}

static ripple__generator ripple__find_generator(const char *name) {
  for (unsigned i = 0; i < sizeof(generators) / sizeof(generators[0]); i++) {
    if (!strcmp(generators[i].name, name))
      return generators[i].gen;
  }
  return NULL;
}

static void ripple__minmax(const double *data, size_t size, double *minp,
                           double *maxp) {
  double min = data[0];
  double max = data[0];
  for (unsigned i = 1; i < size; i++) {
    if (data[i] < min)
      min = data[i];
    if (data[i] > max)
      max = data[i];
  }
  if (minp)
    *minp = min;
  if (maxp)
    *maxp = max;
}

static void ripple__build(filter *filt, const y4m2_parameters *parms,
                          const y4m2_frame *frame) {
  (void)parms;

  ripple__work *wrk = filt->ctx = jd_alloc(sizeof(ripple__work));

  wrk->delay = jd_alloc(sizeof(double) * frame->i.size);

  double amplitude = model_get_real(&filt->config, 10, "$.options.amplitude");
  jd_var *gv = model_get(&filt->config, NULL, "$.options.generator");
  int invert = model_get_int(&filt->config, 0, "$.options.invert");

  if (!gv) {
    fprintf(stderr, "Missing 'generator' in config");
    exit(1);
  }

  ripple__generator gen = ripple__find_generator(jd_bytes(gv, NULL));

  if (!gen) {
    fprintf(stderr, "Can't find generator '%s'", jd_bytes(gv, NULL));
    exit(1);
  }

  /* generate delay pattern */
  gen(filt, frame, wrk->delay);

  /* fill other planes */
  double *delay_plane = wrk->delay + frame->i.plane[0].size;
  for (unsigned p = 1; p < Y4M2_N_PLANE; p++) {
    ripple__resample(delay_plane, wrk->delay, frame->i.width, frame->i.height,
                     frame->i.plane[p].xs, frame->i.plane[p].ys);
    delay_plane += frame->i.plane[p].size;
  }

  /* find min, max delay */
  double min, max;

  if (invert)
    ripple__minmax(wrk->delay, frame->i.size, &max, &min);
  else
    ripple__minmax(wrk->delay, frame->i.size, &min, &max);

  double scale = amplitude / (max - min);

  /* offset all delays */
  for (unsigned i = 0; i < frame->i.size; i++) {
    wrk->delay[i] = (wrk->delay[i] - min) * scale;
  }

  /* allocate frame stores */
  wrk->hist_size = (size_t)(amplitude + 2);
  wrk->history = jd_alloc(sizeof(y4m2_frame *) * wrk->hist_size);

  /* work buffer */
  wrk->buf = y4m2_like_frame(frame);

  wrk->parms = y4m2_clone_parms(parms);
}

static void ripple__start(filter *filt, const y4m2_parameters *parms) {
  y4m2_emit_start(filt->out, parms);
}

static uint8_t pl_fill[Y4M2_N_PLANE] = {16, 128, 128};

static void ripple__frame(filter *filt, const y4m2_parameters *parms,
                          y4m2_frame *frame) {
  if (!filt->ctx)
    ripple__build(filt, parms, frame);

  ripple__work *wrk = filt->ctx;

  /* clock in a new frame */
  y4m2_release_frame(wrk->history[wrk->hist_size - 1]);
  memmove(&wrk->history[1], &wrk->history[0],
          sizeof(y4m2_frame *) * (wrk->hist_size - 1));
  wrk->history[0] = y4m2_retain_frame(frame);

  /* build output frame */
  const double *delp = wrk->delay;
  uint8_t *outp = wrk->buf->buf;
  for (unsigned pl = 0; pl < Y4M2_N_PLANE; pl++) {
    size_t size = wrk->buf->i.plane[pl].size;
    for (unsigned i = 0; i < size; i++) {
      double del = *delp++;
      unsigned index = (unsigned)del;
      double w1 = del - index;
      double w0 = 1 - w1;

      uint8_t s0 = wrk->history[index + 0]
                       ? wrk->history[index + 0]->plane[pl][i]
                       : pl_fill[pl];

      uint8_t s1 = wrk->history[index + 1]
                       ? wrk->history[index + 1]->plane[pl][i]
                       : pl_fill[pl];

      *outp++ = s0 * w0 + s1 * w1;
    }
  }
  y4m2_emit_frame(filt->out, wrk->parms, wrk->buf);
}

static void ripple__end(filter *filt) {
  ripple__work *wrk = filt->ctx;
  for (unsigned i = 0; i < wrk->hist_size; i++)
    ripple__frame(filt, NULL, NULL);
  y4m2_emit_end(filt->out);
  ripple__free(filt->ctx);
}

void ripple_register(void) {
  filter f = {
      .start = ripple__start, .frame = ripple__frame, .end = ripple__end};
  filter_register("ripple", &f);
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
