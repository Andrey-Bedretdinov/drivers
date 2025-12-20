#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "can_eth_uapi.h"

static void print_frame(const struct can_frame_simple *fr)
{
  int i;

  printf("id=0x%X dlc=%u data=", fr->id, fr->dlc);
  for (i = 0; i < fr->dlc; i++) {
    printf("%02X", fr->data[i]);
    if (i + 1 < fr->dlc)
      printf(" ");
  }
  printf("\n");
}

int main(void)
{
  const char *dev = "/dev/can_eth";
  struct can_frame_simple fr;
  int fd;
  ssize_t rd;

  fd = open(dev, O_RDONLY);
  if (fd < 0) {
    perror("open");
    return 1;
  }

  while (1) {
    memset(&fr, 0, sizeof(fr));
    rd = read(fd, &fr, sizeof(fr));
    if (rd < 0) {
      if (errno == EINTR)
        continue;
      perror("read");
      break;
    }

    if (rd == 0) {
      printf("eof\n");
      break;
    }

    if (rd != sizeof(fr)) {
      printf("partial read: %zd\n", rd);
      continue;
    }

    print_frame(&fr);
  }

  close(fd);
  return 0;
}