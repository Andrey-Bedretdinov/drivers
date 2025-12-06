#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define IOCTL_CLEAR _IO('q', 1)
#define IOCTL_HASDATA _IOR('q', 2, int)

int main() {
  const char *dev = "/dev/pz3_dev";
  char buf[256];

  printf("открываем устройство...\n");
  int fd = open(dev, O_RDWR);
  if (fd < 0) {
    printf("не удалось открыть %s\n", dev);
    return 1;
  }

  printf("пишем 'hello' в драйвер\n");
  write(fd, "hello", 5);

  printf("читаем...\n");
  int r = read(fd, buf, sizeof(buf) - 1);
  buf[r] = 0;
  printf("прочитано: %s\n", buf);

  printf("проверяем, есть ли данные через ioctl...\n");
  int has = 0;
  ioctl(fd, IOCTL_HASDATA, &has);
  printf("hasdata=%d\n", has);

  printf("очищаем буфер ioctl CLEAR...\n");
  ioctl(fd, IOCTL_CLEAR);

  has = 0;
  ioctl(fd, IOCTL_HASDATA, &has);
  printf("после очистки hasdata=%d\n", has);

  close(fd);
  return 0;
}