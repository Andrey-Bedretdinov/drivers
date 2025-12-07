- Создать символьный драйвер, который будет автоматически создавать специальный файл устройства.
- Реализовать чтение и запись в глобальный буфер.
- Реализовать ioctl для сброса содержимого буфера и получения информации, есть ли в нём данные.

---

# 1. Сборка драйвера

запускаем make и смотрим что получилось

```shell
parallels@ubuntu-gnu-linux-24-04-3:~/drivers/practice/4$ make
make -C /lib/modules/6.14.0-36-generic/build M=/home/parallels/drivers/practice/4 modules
make[1]: вход в каталог «/usr/src/linux-headers-6.14.0-36-generic»
make[2]: вход в каталог «/home/parallels/drivers/practice/4»
warning: the compiler differs from the one used to build the kernel
  The kernel was built by: aarch64-linux-gnu-gcc-13 (Ubuntu 13.3.0-6ubuntu2~24.04) 13.3.0
  You are using:           gcc-13 (Ubuntu 13.3.0-6ubuntu2~24.04) 13.3.0
  CC [M]  driver.o
  MODPOST Module.symvers
  CC [M]  driver.mod.o
  CC [M]  .module-common.o
  LD [M]  driver.ko
  BTF [M] driver.ko
Skipping BTF generation for driver.ko due to unavailability of vmlinux
make[2]: выход из каталога «/home/parallels/drivers/practice/4»
make[1]: выход из каталога «/usr/src/linux-headers-6.14.0-36-generic»
parallels@ubuntu-gnu-linux-24-04-3:~/drivers/practice/4$ sudo insmod driver.ko
```

# 2. Загрузка модуля

загружаем модуль и проверяем что он в системе

```shell
parallels@ubuntu-gnu-linux-24-04-3:~/drivers/practice/4$ lsmod | grep driver
driver                 12288  0
parallels@ubuntu-gnu-linux-24-04-3:~/drivers/practice/4$ sudo dmesg | tail -1
[ 6445.528247] драйвер загружен, major=234
```

# 3. Создание спецфайла

посмотрим, что появился спецфайл устройства с нужным major

```shell
parallels@ubuntu-gnu-linux-24-04-3:~/drivers/practice/4$ ls -l /dev | grep pz4
crw-------  1 root root    234,   0 дек  7 02:21 pz4_dev
```

# 4. Тестирование через приложение

компилируем и запускаем тестовую программу, проверяем работу драйвера

```shell
parallels@ubuntu-gnu-linux-24-04-3:~/drivers/practice/4$ gcc main.c -o main
parallels@ubuntu-gnu-linux-24-04-3:~/drivers/practice/4$ sudo ./main
открываем устройство...
пишем 'hello' в драйвер
читаем...
прочитано: hello
проверяем, есть ли данные через ioctl...
hasdata=1
очищаем буфер ioctl CLEAR...
после очистки hasdata=0
```

# 5. Логи драйвера

смотрим последние сообщения ядра, связанные с драйвером

```shell
parallels@ubuntu-gnu-linux-24-04-3:~/drivers/practice/4$ sudo dmesg | tail -40
[ 6445.528247] драйвер загружен, major=234
[ 6544.575564] устройство открыто
[ 6544.575576] записано 5 байт
[ 6544.575611] буфер очищен
[ 6544.575614] устройство закрыто
```

# 6. Выгрузка модуля

выгружаем модуль и убеждаемся, что он удалён

```shell
parallels@ubuntu-gnu-linux-24-04-3:~/drivers/practice/4$ sudo rmmod driver
parallels@ubuntu-gnu-linux-24-04-3:~/drivers/practice/4$ lsmod | grep driver
parallels@ubuntu-gnu-linux-24-04-3:~/drivers/practice/4$ sudo dmesg | tail -20
[ 6577.926238] драйвер выгружен
parallels@ubuntu-gnu-linux-24-04-3:~/drivers/practice/4$ 
```