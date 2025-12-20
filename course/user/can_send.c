#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "can_eth_uapi.h"

static int parse_u8_hex(const char *s, uint8_t *out)
{
  char *end = NULL;
  long v;

  errno = 0;
  v = strtol(s, &end, 16);
  if (errno != 0 || end == s || *end != '\0')
    return -1;
  if (v < 0 || v > 255)
    return -1;

  *out = (uint8_t)v;
  return 0;
}

static int parse_u32_hex(const char *s, uint32_t *out)
{
  char *end = NULL;
  unsigned long v;

  errno = 0;
  v = strtoul(s, &end, 16);
  if (errno != 0 || end == s || *end != '\0')
    return -1;

  *out = (uint32_t)v;
  return 0;
}

static int parse_u8_dec(const char *s, uint8_t *out)
{
  char *end = NULL;
  long v;

  errno = 0;
  v = strtol(s, &end, 10);
  if (errno != 0 || end == s || *end != '\0')
    return -1;
  if (v < 0 || v > 8)
    return -1;

  *out = (uint8_t)v;
  return 0;
}

int main(int argc, char **argv)
{
  const char *dev = "/dev/can_eth";
  struct can_frame_simple fr;
  int fd;
  ssize_t wr;
  int i;

  if (argc < 3) {
    fprintf(stderr, "usage:\n");
    fprintf(stderr, "  %s <can_id_hex> <dlc_dec> [data0_hex ... data7_hex]\n", argv[0]);
    fprintf(stderr, "example:\n");
    fprintf(stderr, "  %s 123 3 11 22 33\n", argv[0]);
    return 1;
  }

  memset(&fr, 0, sizeof(fr));

  if (parse_u32_hex(argv[1], &fr.id) != 0) {
    fprintf(stderr, "bad can id: %s\n", argv[1]);
    return 1;
  }

  if (parse_u8_dec(argv[2], &fr.dlc) != 0) {
    fprintf(stderr, "bad dlc: %s\n", argv[2]);
    return 1;
  }

  if (argc != 3 + fr.dlc) {
    fprintf(stderr, "need exactly %u data bytes\n", fr.dlc);
    return 1;
  }

  for (i = 0; i < fr.dlc; i++) {
    if (parse_u8_hex(argv[3 + i], &fr.data[i]) != 0) {
      fprintf(stderr, "bad data byte: %s\n", argv[3 + i]);
      return 1;
    }
  }

  fd = open(dev, O_WRONLY);
  if (fd < 0) {
    perror("open");
    return 1;
  }

  wr = write(fd, &fr, sizeof(fr));
  if (wr < 0) {
    perror("write");
    close(fd);
    return 1;
  }

  if (wr != sizeof(fr)) {
    fprintf(stderr, "partial write: %zd\n", wr);
    close(fd);
    return 1;
  }

  close(fd);
  return 0;
}