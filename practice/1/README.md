# 1. Написать программу для чтения данных с устройства /dev/zero:
```c
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
```

# 2. При помощи strace найти системные вызовы для работы с файловой и подсистемой работы с памятью
```sh
parallels@ubuntu-gnu-linux-24-04-3:/tmp/tmp.hLCpMjeeG4/cmake-build-debug-remote-host$ strace ./1
execve("./1", ["./1"], 0xffffe116f950 /* 35 vars */) = 0
brk(NULL)                               = 0xbbb4ef6fe000
mmap(NULL, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0xf55e58c06000
faccessat(AT_FDCWD, "/etc/ld.so.preload", R_OK) = -1 ENOENT (Нет такого файла или каталога)
openat(AT_FDCWD, "/etc/ld.so.cache", O_RDONLY|O_CLOEXEC) = 3
fstat(3, {st_mode=S_IFREG|0644, st_size=56863, ...}) = 0
mmap(NULL, 56863, PROT_READ, MAP_PRIVATE, 3, 0) = 0xf55e58bf8000
close(3)                                = 0
openat(AT_FDCWD, "/lib/aarch64-linux-gnu/libc.so.6", O_RDONLY|O_CLOEXEC) = 3
read(3, "\177ELF\2\1\1\3\0\0\0\0\0\0\0\0\3\0\267\0\1\0\0\0\360\206\2\0\0\0\0\0"..., 832) = 832
fstat(3, {st_mode=S_IFREG|0755, st_size=1722920, ...}) = 0
mmap(NULL, 1892240, PROT_NONE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_DENYWRITE, -1, 0) = 0xf55e58a00000
mmap(0xf55e58a00000, 1826704, PROT_READ|PROT_EXEC, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0) = 0xf55e58a00000
munmap(0xf55e58bbe000, 65424)           = 0
mprotect(0xf55e58b99000, 81920, PROT_NONE) = 0
mmap(0xf55e58bad000, 20480, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x19d000) = 0xf55e58bad000
mmap(0xf55e58bb2000, 49040, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_ANONYMOUS, -1, 0) = 0xf55e58bb2000
close(3)                                = 0
set_tid_address(0xf55e58c06fb0)         = 10451
set_robust_list(0xf55e58c06fc0, 24)     = 0
rseq(0xf55e58c07600, 0x20, 0, 0xd428bc00) = 0
mprotect(0xf55e58bad000, 12288, PROT_READ) = 0
mprotect(0xbbb4c891f000, 4096, PROT_READ) = 0
mprotect(0xf55e58c0c000, 8192, PROT_READ) = 0
prlimit64(0, RLIMIT_STACK, NULL, {rlim_cur=8192*1024, rlim_max=RLIM64_INFINITY}) = 0
munmap(0xf55e58bf8000, 56863)           = 0
fstat(1, {st_mode=S_IFCHR|0620, st_rdev=makedev(0x88, 0), ...}) = 0
getrandom("\x8b\x04\xcc\xb1\x23\x29\xb9\x5d", 8, GRND_NONBLOCK) = 8
brk(NULL)                               = 0xbbb4ef6fe000
brk(0xbbb4ef71f000)                     = 0xbbb4ef71f000
write(1, "\320\276\321\202\320\272\321\200\321\213\320\262\320\260\320\265\320\274 \321\203\321\201\321\202\321\200\320\276\320\271\321"..., 53открываем устройство /dev/zero...
) = 53
openat(AT_FDCWD, "/dev/zero", O_RDONLY) = 3
write(1, "\321\204\320\260\320\271\320\273 \320\276\321\202\320\272\321\200\321\213\321\202. \320\224\320\265\321\201\320\272\321"..., 47файл открыт. Дескриптор: 3
) = 47
write(1, "\321\207\320\270\321\202\320\260\320\265\320\274 100 \320\261\320\260\320\271\321\202 \320\270\320\267 /"..., 44читаем 100 байт из /dev/zero...
) = 44
read(3, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"..., 100) = 100
write(1, "\320\277\321\200\320\276\321\207\320\270\321\202\320\260\320\275\320\276 \320\261\320\260\320\271\321\202: 100"..., 33прочитано байт: 100
) = 33
write(1, "\320\267\320\260\320\272\321\200\321\213\320\262\320\260\320\265\320\274 \321\204\320\260\320\271\320\273...\n", 31закрываем файл...
) = 31
close(3)                                = 0
write(1, "\320\236\320\232\n", 5ОК
)       = 5
exit_group(0)                           = ?
+++ exited with 0 +++
```

Подсистема работы с памятью:
- brk — изменение границ сегмента данных процесса
- mmap — выделение анонимных областей памяти
- munmap — освобождение областей
- mprotect — установка прав доступа к страницам

Файловая подсистема:
- openat — открытие устройства /dev/zero
- read — чтение 100 байт нулей
- write — вывод строк в stdout
- close — закрытие файлового дескриптора
- fstat — получение информации о файле
- faccessat — проверка доступа к системным файлам