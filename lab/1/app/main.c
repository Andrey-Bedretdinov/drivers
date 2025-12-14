#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define IOCTL_GET_HIST_LEN _IOR('x', 1, int)
#define IOCTL_GET_HIST_BUF _IOR('x', 2, size_t[20])

int main(void)
{
  int reader_fd = open("/dev/lab1dev", O_RDONLY);
  if (reader_fd < 0) {
    perror("ошибка открытия reader_fd");
    return 1;
  }

  int writer_fd = open("/dev/lab1dev", O_WRONLY);
  if (writer_fd < 0) {
    perror("ошибка открытия writer_fd");
    close(reader_fd);
    return 1;
  }

  for (int i = 0; i < 1000; i++) {
    write(writer_fd, &i, sizeof(int));

    int r;
    read(reader_fd, &r, sizeof(int));
  }

  int hist_len = 0;
  ioctl(reader_fd, IOCTL_GET_HIST_LEN, &hist_len);

  size_t hist_buf[20];
  ioctl(reader_fd, IOCTL_GET_HIST_BUF, hist_buf);

  printf("Гистограмма (%d бинов):\n", hist_len);

  for (int i = 0; i < hist_len; i++)
    printf("Бин %02d: %ld\n", i, hist_buf[i]);

  close(reader_fd);
  close(writer_fd);
  return 0;
}
