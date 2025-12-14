#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define IOCTL_GET_HIST_LEN _IOR('x', 1, int)
#define IOCTL_GET_HIST_BUF _IOR('x', 2, size_t[20])

int main(void)
{
  int fd = open("/dev/lab1dev", O_RDWR);
  if (fd < 0) {
    perror("open");
    return 1;
  }

  for (int i = 0; i < 1000; i++) {
    write(fd, &i, sizeof(int));

    int r;
    read(fd, &r, sizeof(int));
  }

  int hist_len = 0;
  ioctl(fd, IOCTL_GET_HIST_LEN, &hist_len);

  size_t hist_buf[20];
  ioctl(fd, IOCTL_GET_HIST_BUF, hist_buf);

  printf("Histogram (%d bins):\n", hist_len);

  for (int i = 0; i < hist_len; i++)
    printf("%02d: %ld\n", i, hist_buf[i]);

  close(fd);
  return 0;
}
