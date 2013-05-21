/* yuv4mpeg2.h */

#ifndef YUV4MPEG2_H_
#define YUV4MPEG2_H_

#include <stdint.h>
#include <stdio.h>

#define Y4M2_FIRST 'A'
#define Y4M2_LAST 'Z'
#define Y4M2_PARMS (Y4M2_LAST-Y4M2_FIRST+1)

typedef struct {
  char *parm[Y4M2_PARMS];
} y4m2_parameters;

enum { Y4M2_Y_PLANE , Y4M2_Cb_PLANE , Y4M2_Cr_PLANE, Y4M2_N_PLANE };

typedef struct {
  unsigned width;
  unsigned height;
  size_t size;
  struct {
    unsigned xs, ys;
    size_t size;
  } plane[Y4M2_N_PLANE];
} y4m2_frame_info;

typedef struct {
  y4m2_frame_info i;
  uint8_t *buf;
  uint8_t *plane[Y4M2_N_PLANE];
} y4m2_frame;

typedef enum { Y4M2_START, Y4M2_FRAME } y4m2_reason;

typedef void (*y4m2_callback)(y4m2_reason reason,
                              y4m2_parameters *parms,
                              y4m2_frame *frame,
                              void *ctx);

void y4m2__parse_parms(y4m2_parameters *parms, char *buf);
void y4m2__format_parms(FILE *out, const y4m2_parameters *parms);

y4m2_parameters *y4m2_new_parms(void);
void y4m2_free_parms(y4m2_parameters *parms);
y4m2_parameters *y4m2_clone_parms(const y4m2_parameters *orig);
const char *y4m2_get_parm(const y4m2_parameters *parms, const char *name);
void y4m2_set_parm(y4m2_parameters *parm, const char *name, const char *value);
void y4m2_parse_frame_info(y4m2_frame_info *info, const y4m2_parameters *parms);
y4m2_frame *y4m2_new_frame(const y4m2_parameters *parms);
void y4m2_free_frame(y4m2_frame *frame);
int y4m2_parse(FILE *in, y4m2_callback cb, void *ctx);
int y4m2_emit_start(FILE *out, const y4m2_parameters *parms);
int y4m2_emit_frame(FILE *out, const y4m2_parameters *parms, const y4m2_frame *frame);

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
