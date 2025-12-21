#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

struct can_frame_simple {
  uint32_t id;
  uint8_t  dlc;
  uint8_t  data[8];
};

static void print_frame(const struct can_frame_simple *f)
{
  unsigned i;

  printf("id=0x%X dlc=%u data=", f->id, f->dlc);
  for (i = 0; i < f->dlc; i++) {
    printf("%02X", f->data[i]);
    if (i + 1 < f->dlc)
      printf(" ");
  }
  printf("\n");
}

int main(void)
{
  struct can_frame_simple f;
  int fd;
  ssize_t rd;

  fd = open("/dev/can_eth", O_RDONLY);
  if (fd < 0) {
    perror("open");
    return 1;
  }

  while (1) {
    memset(&f, 0, sizeof(f));

    rd = read(fd, &f, sizeof(f));
    if (rd < 0) {
      if (errno == EINTR)
        continue;
      perror("read");
      break;
    }

    if (rd != sizeof(f)) {
      printf("partial read: %zd\n", rd);
      continue;
    }

    if (f.dlc > 8) {
      printf("bad dlc=%u\n", f.dlc);
      continue;
    }

    print_frame(&f);
  }

  close(fd);
  return 0;
}