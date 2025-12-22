#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// структура can кадра в том же виде, как ожидает драйвер
struct can_frame_simple {
  uint32_t id;
  uint8_t dlc;
  uint8_t data[8];
};

int main(int argc, char **argv) {
  // структура для отправляемого кадра
  struct can_frame_simple f;

  // файловый дескриптор устройства
  int fd;

  // счётчик для заполнения data
  int i;

  // результат записи в устройство
  ssize_t wr;

  // проверяем, что передали минимум id и dlc
  if (argc < 3) {
    fprintf(stderr, "usage: %s <id_hex> <dlc_dec> [bytes_hex...]\n", argv[0]);
    return 1;
  }

  // обнуляем структуру кадра
  memset(&f, 0, sizeof(f));

  // парсим id кадра из hex строки
  f.id = (uint32_t)strtoul(argv[1], NULL, 16);

  // парсим длину данных
  f.dlc = (uint8_t)atoi(argv[2]);

  // проверяем, что dlc не больше 8
  if (f.dlc > 8) {
    fprintf(stderr, "dlc must be 0..8\n");
    return 1;
  }

  // проверяем, что количество байт совпадает с dlc
  if (argc != 3 + f.dlc) {
    fprintf(stderr, "need exactly %u data bytes\n", (unsigned)f.dlc);
    return 1;
  }

  // заполняем массив данных из аргументов командной строки
  for (i = 0; i < f.dlc; i++)
    f.data[i] = (uint8_t)strtoul(argv[3 + i], NULL, 16);

  // открываем файл устройства на запись
  fd = open("/dev/can_eth", O_WRONLY);
  if (fd < 0) {
    perror("open");
    return 1;
  }

  // записываем can кадр в драйвер
  wr = write(fd, &f, sizeof(f));
  if (wr < 0) {
    perror("write");
    close(fd);
    return 1;
  }

  // проверяем, что записался весь кадр целиком
  if (wr != sizeof(f)) {
    fprintf(stderr, "partial write: %zd\n", wr);
    close(fd);
    return 1;
  }

  close(fd);
  return 0;
}