#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

struct can_frame_simple {
  uint32_t id;
  uint8_t  dlc;
  uint8_t  data[8];
};

int main(int argc, char **argv)
{
  struct can_frame_simple f;
  int fd;
  int i;
  ssize_t wr;

  if (argc < 3) {
    fprintf(stderr, "usage: %s <id_hex> <dlc_dec> [bytes_hex...]\n", argv[0]);
    return 1;
  }

  memset(&f, 0, sizeof(f));
  f.id = (uint32_t)strtoul(argv[1], NULL, 16);
  f.dlc = (uint8_t)atoi(argv[2]);

  if (f.dlc > 8) {
    fprintf(stderr, "dlc must be 0..8\n");
    return 1;
  }

  if (argc != 3 + f.dlc) {
    fprintf(stderr, "need exactly %u data bytes\n", (unsigned)f.dlc);
    return 1;
  }

  for (i = 0; i < f.dlc; i++)
    f.data[i] = (uint8_t)strtoul(argv[3 + i], NULL, 16);

  fd = open("/dev/can_eth", O_WRONLY);
  if (fd < 0) {
    perror("open");
    return 1;
  }

  wr = write(fd, &f, sizeof(f));
  if (wr < 0) {
    perror("write");
    close(fd);
    return 1;
  }

  if (wr != sizeof(f)) {
    fprintf(stderr, "partial write: %zd\n", wr);
    close(fd);
    return 1;
  }

  close(fd);
  return 0;
}