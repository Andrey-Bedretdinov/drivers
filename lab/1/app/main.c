#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define IOCTL_GET_HIST_LEN _IOR('x', 1, int)
#define IOCTL_GET_HIST_BUF _IOR('x', 2, size_t[500])

int main() {
  const char *path = "/dev/lab1dev";

  int reader_fd = open(path, O_RDONLY);
  if (reader_fd < 0) {
    perror("Ошибка открытия reader_fd");
    return 1;
  }

  int writer_fd = open(path, O_WRONLY);
  if (writer_fd < 0) {
    perror("Ошибка открытия writer_fd");
    close(reader_fd);
    return 1;
  }

  for (size_t i = 0; i < 1000; i++) {
    write(writer_fd, &i, sizeof(int));
    usleep(50);
    read(reader_fd, &i, sizeof(int));
  }

  size_t hist_len = 0;
  ioctl(reader_fd, IOCTL_GET_HIST_LEN, &hist_len);

  printf("Гистограмма, длина = %zu\n", hist_len);

  size_t *buf = malloc(hist_len * sizeof(size_t));
  ioctl(reader_fd, IOCTL_GET_HIST_BUF, buf);

  for (size_t i = 0; i < hist_len; i++)
    printf("%zu:\t%zu\n", i, buf[i]);

  free(buf);
  close(reader_fd);
  close(writer_fd);

  return 0;
}