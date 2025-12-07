#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int main() {
  const char* dev = "/dev/foo_device";

  printf("открываем устройство: %s\n", dev);

  int fd = open(dev, O_RDWR);
  if (fd < 0) {
    printf("не удалось открыть устройство\n");
    return 1;
  }

  printf("устройство открыто успешно, fd=%d\n", fd);

  close(fd);
  return 0;
}