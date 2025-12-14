#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define IOCTL_GET_HIST_LEN _IOR('x', 1, int)
#define IOCTL_GET_HIST_BUF _IOR('x', 2, size_t[20])

int main() {
  const char *path = "/dev/lab1dev";

  // открываю устройство для чтения
  int reader_fd = open(path, O_RDONLY);
  // открываю устройство для записи
  int writer_fd = open(path, O_WRONLY);

  // делаю 1000 операций запись чтение
  for (size_t i = 0; i < 1000; i++) {
    write(writer_fd, &i, sizeof(int));
    usleep(1);
    read(reader_fd, &i, sizeof(int));
  }

  // получаю длину гистограммы
  int hist_len = 0;
  ioctl(reader_fd, IOCTL_GET_HIST_LEN, &hist_len);

  printf("Гистограмма, длина = %d\n", hist_len);

  size_t hist[20] = {0};
  // получаю буфер гистограммы
  ioctl(reader_fd, IOCTL_GET_HIST_BUF, hist);

  // вывожу гистограмму
  for (int i = 0; i < hist_len; i++)
    printf("%02d:\t%zu\n", i, hist[i]);

  // закрываю дескрипторы
  close(reader_fd);
  close(writer_fd);
}