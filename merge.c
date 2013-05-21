/* merge.c */

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BMP_HEADER_SIZE 14
#define DIB_HEADER_SIZE 40

static void die(const char *msg, ...) {
  va_list ap;
  va_start(ap, msg);
  fprintf(stderr, "Fatal: ");
  vfprintf(stderr, msg, ap);
  fprintf(stderr, "\n");
  exit(1);
}

static uint16_t read16(uint8_t *d) {
  return (d[0] << 0) |
         (d[1] << 8);
}

static uint32_t read32(uint8_t *d) {
  return (d[0] << 0) |
         (d[1] << 8) |
         (d[2] << 16) |
         (d[3] << 24);
}

static void write16(uint8_t *d, unsigned idx, uint16_t v) {
  d[idx + 0] = (v >> 0) & 0xFF;
  d[idx + 1] = (v >> 8) & 0xFF;
}

static void write32(uint8_t *d, unsigned idx, uint32_t v) {
  d[idx + 0] = (v >> 0) & 0xFF;
  d[idx + 1] = (v >> 8) & 0xFF;
  d[idx + 2] = (v >> 16) & 0xFF;
  d[idx + 3] = (v >> 24) & 0xFF;
}

static void parse_bmp(uint8_t *hdr, uint32_t *size, uint32_t *data) {
  *size = read32(hdr + 0x02);
  *data = read32(hdr + 0x0a);
}

static uint32_t parse_dib(uint8_t *hdr, uint32_t *width, uint32_t *height, uint16_t *bpp) {
  uint32_t size = read32(hdr + 0x00);
  *width = read32(hdr + 0x04);
  *height = read32(hdr + 0x08);
  *bpp = read16(hdr + 0x0e);
  return size;
}

static void *alloc(size_t size) {
  void *m = malloc(size);
  if (!m) die("Can't allocated %lu bytes", (unsigned long) size);
  memset(m, 0, size);
  return m;
}

static void skip(int fd, uint32_t *pos, uint32_t data) {
  uint8_t buf[65536];
  ssize_t len = data - *pos;
  while (len > 0) {
    size_t want = len;
    if (want > sizeof(buf)) want = sizeof(buf);
    ssize_t got = read(fd, buf, want);
    if (got < 0) die("Read error: %m");
    len -= got;
  }
  *pos = data;
}

static void read_buf(int fd, uint8_t *buf, uint32_t len) {
  while (len) {
    ssize_t got = read(fd, buf, len);
    if (got < 0) die("Read error: %m");
    if (got == 0) die("Unexpected EOF");
    buf += got;
    len -= got;
  }
}

static void write_buf(int fd, uint8_t *buf, uint32_t len) {
  ssize_t got = write(fd, buf, len);
  if (got < 0) die("Write error: %m");
  if (got < len) die("Short write");
}

static void add_buf(uint32_t *img, uint8_t *buf, uint32_t size) {
  uint32_t i;
  for (i = 0; i < size; i++)
    img[i] += buf[i];
}

static void average(uint8_t *buf, uint32_t *img, uint32_t size, unsigned count) {
  uint32_t i;
  for (i = 0; i < size; i++)
    buf[i] = img[i] / count;
}

int main(int argc, char *argv[]) {
  unsigned frames = 200;
  unsigned phase = 0;
  uint8_t bmp_hdr[BMP_HEADER_SIZE];
  uint8_t dib_hdr[DIB_HEADER_SIZE];
  uint32_t *image = NULL;
  uint8_t *buf = NULL;

  if (argc > 1) frames = atoi(argv[1]);
  if (frames == 0) die("Invalid frame count: %s", argv[1]);

  for (;;) {
    uint32_t size, data;
    uint32_t width, height;
    uint16_t bpp;
    uint32_t pos = BMP_HEADER_SIZE + DIB_HEADER_SIZE;

    ssize_t got = read(0, bmp_hdr, BMP_HEADER_SIZE);
    if (got < 0) die("Read error: %m");
    if (got == 0) break;
    if (got < BMP_HEADER_SIZE) die("Short read");
    if (bmp_hdr[0] != 'B' || bmp_hdr[1] != 'M') die("Not a bitmap");
    parse_bmp(bmp_hdr, &size, &data);

    got = read(0, dib_hdr, DIB_HEADER_SIZE);
    if (got < 0) die("Read error: %m");
    if (got < DIB_HEADER_SIZE) die("Short read");
    parse_dib(dib_hdr, &width, &height, &bpp);

    size_t bytes = width * height * bpp / 8;
    if (NULL == buf) buf = alloc(bytes);
    if (NULL == image) image = alloc(bytes * sizeof(uint32_t));

    /*    fprintf(stderr, "[%04u] width=%lu, height=%lu, bpp=%lu\n", phase, width, height, bpp);*/

    skip(0, &pos, data);

    read_buf(0, buf, bytes);
    pos += bytes;
    add_buf(image, buf, bytes);

    skip(0, &pos, size);

    if (++phase == frames) {
      average(buf, image, bytes, frames);
      write32(bmp_hdr, 0x02, BMP_HEADER_SIZE + DIB_HEADER_SIZE + bytes);
      write32(bmp_hdr, 0x0a, BMP_HEADER_SIZE + DIB_HEADER_SIZE);
      write32(dib_hdr, 0x00, DIB_HEADER_SIZE);

      write_buf(1, bmp_hdr, BMP_HEADER_SIZE);
      write_buf(1, dib_hdr, DIB_HEADER_SIZE);
      write_buf(1, buf, bytes);

      memset(image, 0, bytes * sizeof(uint32_t));

      phase = 0;
    }
  }

  return 0;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
