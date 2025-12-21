#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>

struct can_frame_simple {
  uint32_t id;
  uint8_t  dlc;
  uint8_t  data[8];
};

int main(void)
{
  struct can_frame_simple f;
  int fd, i;

  fd = open("/dev/can_eth", O_RDONLY);

  while (1) {
    read(fd, &f, sizeof(f));
    printf("id=%x dlc=%u data=", f.id, f.dlc);
    for (i = 0; i < f.dlc; i++)
      printf("%02x ", f.data[i]);
    printf("\n");
  }
}