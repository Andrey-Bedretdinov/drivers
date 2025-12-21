#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct can_frame_simple {
  uint32_t id;
  uint8_t  dlc;
  uint8_t  data[8];
};

int main(int argc, char **argv)
{
  struct can_frame_simple f = {};
  int fd;
  int i;

  if (argc < 3)
    return 1;

  f.id = strtoul(argv[1], NULL, 16);
  f.dlc = atoi(argv[2]);

  for (i = 0; i < f.dlc; i++)
    f.data[i] = strtoul(argv[3 + i], NULL, 16);

  fd = open("/dev/can_eth", O_WRONLY);
  write(fd, &f, sizeof(f));
  close(fd);
  return 0;
}