#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

// структура can кадра в том же формате, что и в драйвере
struct can_frame_simple {
  uint32_t id;
  uint8_t dlc;
  uint8_t data[8];
};

// функция печати can кадра
static void print_frame(const struct can_frame_simple *f) {
  // счётчик для перебора байт данных
  unsigned i;

  // выводим id и длину данных
  printf("id=0x%X dlc=%u data=", f->id, f->dlc);

  // выводим каждый байт данных в hex формате
  for (i = 0; i < f->dlc; i++) {
    printf("%02X", f->data[i]);
    if (i + 1 < f->dlc)
      printf(" ");
  }

  // перевод строки после вывода кадра
  printf("\n");
}

int main(void) {
  // структура для принятого can кадра
  struct can_frame_simple f;

  // файловый дескриптор устройства
  int fd;

  // результат операции чтения
  ssize_t rd;

  // открываем файл устройства на чтение
  fd = open("/dev/can_eth", O_RDONLY);
  if (fd < 0) {
    perror("open");
    return 1;
  }

  // основной цикл приёма кадров
  while (1) {
    // очищаем структуру перед чтением
    memset(&f, 0, sizeof(f));

    // читаем can кадр из драйвера
    rd = read(fd, &f, sizeof(f));
    if (rd < 0) {
      // если чтение прервано сигналом, пробуем снова
      if (errno == EINTR)
        continue;

      // выводим ошибку чтения
      perror("read");
      break;
    }

    // проверяем, что считался весь кадр целиком
    if (rd != sizeof(f)) {
      printf("partial read: %zd\n", rd);
      continue;
    }

    // проверяем корректность длины данных
    if (f.dlc > 8) {
      printf("bad dlc=%u\n", f.dlc);
      continue;
    }

    // выводим принятый can кадр
    print_frame(&f);
  }

  close(fd);
  return 0;
}