- Скомпилировать символьный драйвер
- Вставить в ядро
- Создать специальный файл устройства
- Написать приложение для открытия специального файла устройства

---

## 1. Сборка символьного драйвера

Команда `make` запускает сборку модуля ядра:

```shell
parallels@ubuntu-gnu-linux-24-04-3:~/drivers/practice/3$ make
make -C /lib/modules/6.14.0-36-generic/build M=/home/parallels/drivers/practice/3 modules
make[1]: вход в каталог «/usr/src/linux-headers-6.14.0-36-generic»
make[2]: вход в каталог «/home/parallels/drivers/practice/3»
warning: the compiler differs from the one used to build the kernel
  The kernel was built by: aarch64-linux-gnu-gcc-13 (Ubuntu 13.3.0-6ubuntu2~24.04) 13.3.0
  You are using:           gcc-13 (Ubuntu 13.3.0-6ubuntu2~24.04) 13.3.0
  CC [M]  hello.o
  MODPOST Module.symvers
  CC [M]  hello.mod.o
  CC [M]  .module-common.o
  LD [M]  hello.ko
  BTF [M] hello.ko
Skipping BTF generation for hello.ko due to unavailability of vmlinux
make[2]: выход из каталога «/home/parallels/drivers/practice/3»
make[1]: выход из каталога «/usr/src/linux-headers-6.14.0-36-generic»
```

---

## 2. Вставка драйвера в ядро

```shell
parallels@ubuntu-gnu-linux-24-04-3:~/drivers/practice/3$ sudo insmod hello.ko
```

Проверка, что модуль загружен:

```shell
parallels@ubuntu-gnu-linux-24-04-3:~/drivers/practice/3$ lsmod | grep hello
hello                  12288  0
```

Логи ядра показывают загрузку:

```shell
parallels@ubuntu-gnu-linux-24-04-3:~/drivers/practice/3$ sudo dmesg | tail
[ 9683.447514] Hello world 1.
```

---

## 3. Посмотреть major-номер драйвера и создать файл устройства

Находим major:

```shell
parallels@ubuntu-gnu-linux-24-04-3$ cat /proc/devices | grep foo
234 foo
```

Создаём спецфайл:

```shell
parallels@ubuntu-gnu-linux-24-04-3$ sudo mknod /dev/foo_device c 234 0
parallels@ubuntu-gnu-linux-24-04-3$ sudo chmod 666 /dev/foo_device
```

---

## 4. Скомпилировать приложение и открыть устройство

Сборка:

```shell
parallels@ubuntu-gnu-linux-24-04-3$ gcc main.c -o main
```

Запуск:

```shell
parallels@ubuntu-gnu-linux-24-04-3$ sudo ./main
открываем устройство: /dev/foo_device
устройство открыто успешно, fd=3
```

Логи ядра фиксируют вызовы open/release:

```shell
parallels@ubuntu-gnu-linux-24-04-3$ sudo dmesg | tail
[ 9683.447514] Hello world 1.
```

---

## 5. Выгрузка драйвера

```shell
parallels@ubuntu-gnu-linux-24-04-3$ sudo rmmod hello
```

Проверка:

```shell
parallels@ubuntu-gnu-linux-24-04-3$ lsmod | grep hello
# пусто — значит выгружен
```

Логи:

```shell
[ 9917.997834] Goodbye world 1.
```



