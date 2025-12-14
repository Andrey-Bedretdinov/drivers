#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define IOCTL_GET_HIST_LEN _IOR('x', 1, int)
#define IOCTL_GET_HIST_BUF _IOR('x', 2, size_t[20])

int main() {
  const char *path = "/dev/lab1dev";

  int reader_fd = open(path, O_RDONLY);
  int writer_fd = open(path, O_WRONLY);

  for (size_t i = 0; i < 1000; i++) {
    write(writer_fd, &i, sizeof(int));
    usleep(60); // <<< ВАЖНО
    read(reader_fd, &i, sizeof(int));
  }

  int hist_len = 0;
  ioctl(reader_fd, IOCTL_GET_HIST_LEN, &hist_len);

  printf("Гистограмма, длина = %d\n", hist_len);

  size_t hist[20] = {0};
  ioctl(reader_fd, IOCTL_GET_HIST_BUF, hist);

  for (int i = 0; i < hist_len; i++)
    printf("%02d:\t%zu\n", i, hist[i]);

  close(reader_fd);
  close(writer_fd);
}