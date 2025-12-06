#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int main() {
    char data[100];

    printf("открываем устройство /dev/zero...\n");
    int file = open("/dev/zero", O_RDONLY);
    printf("файл открыт. Дескриптор: %d\n", file);

    printf("читаем 100 байт из /dev/zero...\n");
    ssize_t bytes_read = read(file, data, sizeof(data));
    printf("прочитано байт: %zd\n", bytes_read);

    printf("закрываем файл...\n");
    close(file);

    printf("ОК\n");

    return 0;
}