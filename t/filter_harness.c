/* filter_harness.c */

#include <jd_pretty.h>
#include <stdint.h>

#include "yuv4mpeg2.h"
#include "filter_harness.h"

typedef struct {
  jd_var capture;
} fh__work;

static void fh__fill_info(y4m2_frame_info *info) {
  info->width = FH_WIDTH;
  info->height = FH_HEIGHT;

  info->plane[0].xs = FH_WIDTH / FH_Y_WIDTH;
  info->plane[0].ys = FH_HEIGHT / FH_Y_HEIGHT;
  info->plane[1].xs = FH_WIDTH / FH_Cb_WIDTH;
  info->plane[1].ys = FH_HEIGHT / FH_Cb_HEIGHT;
  info->plane[2].xs = FH_WIDTH / FH_Cr_WIDTH;
  info->plane[2].ys = FH_HEIGHT / FH_Cr_HEIGHT;

  info->size = 0;
  for (unsigned i = 0; i < Y4M2_N_PLANE; i++) {
    info->plane[i].size = (info->width * info->height) /
                          (info->plane[i].xs * info->plane[i].ys);
    info->size += info->plane[i].size;
  }
}

static void fh__frame(y4m2_frame *frame, fh__work *wrk) {
  scope {
    jd_var *fr = jd_nav(Y4M2_N_PLANE);
    uint8_t *fp = frame->buf;
    for (unsigned p = 0; p < Y4M2_N_PLANE; p++) {
      jd_var *pix = jd_set_array(jd_push(fr, 1), frame->i.plane[p].size);
      for (unsigned i = 0; i < frame->i.plane[p].size; i++)
        jd_set_int(jd_push(pix, 1), *fp++);
    }
    jd_assign(jd_push(&wrk->capture, 1), fr);
  }
}

static void fh__callback(y4m2_reason reason,
                         const y4m2_parameters *parms,
                         y4m2_frame *frame,
                         void *ctx) {
  fh__work *wrk = (fh__work *) ctx;
  (void) parms;
  (void) frame;
  switch (reason) {
  case Y4M2_START:
    break;
  case Y4M2_FRAME:
    fh__frame(frame, wrk);
    break;
  case Y4M2_END:
    jd_release(&wrk->capture);
    jd_free(wrk);
    break;
  }
}

static y4m2_frame *fh__new_frame(void) {
  y4m2_frame_info info;
  fh__fill_info(&info);
  return y4m2_new_frame_info(&info);
}

y4m2_output *fh_get_output(jd_var *capture) {
  fh__work *wrk = jd_alloc(sizeof(fh__work));
  jd_set_array(&wrk->capture, 100);
  if (capture) jd_assign(capture, &wrk->capture);
  return y4m2_output_next(fh__callback, wrk);
}

void fh_push_frame(y4m2_output *out, uint8_t Y, uint8_t Cb, uint8_t Cr) {
  y4m2_frame *frame = fh__new_frame();
  y4m2_parameters *parms = y4m2_new_parms();

  uint8_t samp[Y4M2_N_PLANE];
  samp[Y4M2_Y_PLANE] = Y;
  samp[Y4M2_Cb_PLANE] = Cb;
  samp[Y4M2_Cr_PLANE] = Cr;

  uint8_t *fp = frame->buf;
  for (unsigned p = 0; p < Y4M2_N_PLANE; p++)
    for (unsigned i = 0; i < frame->i.plane[p].size; i++)
      *fp++ = samp[p];

  y4m2_emit_frame(out, parms, frame);

  y4m2_free_parms(parms);
  y4m2_release_frame(frame);
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
