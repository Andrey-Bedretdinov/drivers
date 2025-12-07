Разработать сетевой драйвер, который должен:

- открываться
- иметь MAC-адрес
- иметь возможность имитировать отправку пакетов (т.е. пакет должен получаться в hard_start_xmit)
- для полученного пакета должно при помощи pr_info печататься сообщение

---

## 1. Сборка сетевого драйвера

Запускаю `make`, собирается `net_driver.ko`:

```shell
parallels@ubuntu-gnu-linux-24-04-3:~/drivers/practice/5$ make
make -C /lib/modules/6.14.0-36-generic/build M=/home/parallels/drivers/practice/5 modules
make[1]: вход в каталог «/usr/src/linux-headers-6.14.0-36-generic»
make[2]: вход в каталог «/home/parallels/drivers/practice/5»
warning: the compiler differs from the one used to build the kernel
  The kernel was built by: aarch64-linux-gnu-gcc-13 (Ubuntu 13.3.0-6ubuntu2~24.04) 13.3.0
  You are using:           gcc-13 (Ubuntu 13.3.0-6ubuntu2~24.04) 13.3.0
  CC [M]  net_driver.o
  MODPOST Module.symvers
  CC [M]  net_driver.mod.o
  CC [M]  .module-common.o
  LD [M]  net_driver.ko
  BTF [M] net_driver.ko
Skipping BTF generation for net_driver.ko due to unavailability of vmlinux
make[2]: выход из каталога «/home/parallels/drivers/practice/5»
make[1]: выход из каталога «/usr/src/linux-headers-6.14.0-36-generic»
```

---

## 2. Загрузка модуля и проверка интерфейса

Загружаю модуль:

```shell
parallels@ubuntu-gnu-linux-24-04-3:~/drivers/practice/5$ sudo insmod net_driver.ko
```

Проверяю — интерфейс появился:

```shell
parallels@ubuntu-gnu-linux-24-04-3:~/drivers/practice/5$ ip link show | grep netdemo
3: netdemo0: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc fq_codel state UNKNOWN mode DEFAULT group default qlen 1000
```

Включаю интерфейс:

```shell
parallels@ubuntu-gnu-linux-24-04-3:~/drivers/practice/5$ sudo ip link set netdemo0 up
```

Смотрю MAC‑адрес:

```shell
parallels@ubuntu-gnu-linux-24-04-3:~/drivers/practice/5$ ip link show netdemo0
3: netdemo0: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc fq_codel state UNKNOWN mode DEFAULT group default qlen 1000
    link/ether 5a:83:43:3b:e9:53 brd ff:ff:ff:ff:ff:ff
```

---

## 3. Сборка и запуск программы, отправляющей RAW Ethernet кадр

Собираю:

```shell
parallels@ubuntu-gnu-linux-24-04-3:~/drivers/practice/5$ gcc rawexample.c -o rawsend
```

Запускаю:

```shell
parallels@ubuntu-gnu-linux-24-04-3:~/drivers/practice/5$ sudo ./rawsend
Success!
```

---

## 4. Проверка вывода драйвера — пакет пойман и напечатан в hard_start_xmit

```shell
parallels@ubuntu-gnu-linux-24-04-3:~/drivers/practice/5$ sudo dmesg | tail -50
[11705.780870] netdemo packet: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
[11705.780873] netdemo packet: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
[11705.780876] netdemo packet: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
[11705.780878] netdemo packet: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
[11705.780881] netdemo packet: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
[11705.780884] netdemo packet: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
[11705.780887] netdemo packet: 00 00 00 00 00 00 63 82 53 63 35 01 01 3d 07 01  ......c.Sc5..=..
[11705.780890] netdemo packet: 5a 83 43 3b e9 53 37 11 01 02 06 0c 0f 1a 1c 79  Z.C;.S7........y
[11705.780893] netdemo packet: 03 21 28 29 2a 77 f9 fc 11 39 02 02 40 0c 18 75  .!()*w...9..@..u
[11705.780896] netdemo packet: 62 75 6e 74 75 2d 67 6e 75 2d 6c 69 6e 75 78 2d  buntu-gnu-linux-
[11705.780899] netdemo packet: 32 34 2d 30 34 2d 33 ff                          24-04-3.
[11705.975873] netdemo: packet received in hard_start_xmit, len=25
[11705.975884] netdemo packet: 00 12 34 56 78 90 5a 83 43 3b e9 53 12 34 68 65  ..4Vx.Z.C;.S.4he
[11705.975888] netdemo packet: 6c 6c 6f 20 77 6f 72 6c 64                       llo world
[11718.557121] netdemo: packet received in hard_start_xmit, len=62
[11718.557140] netdemo packet: 33 33 00 00 00 02 5a 83 43 3b e9 53 86 dd 60 09  33....Z.C;.S..`.
[11718.557147] netdemo packet: eb ac 00 08 3a ff fe 80 00 00 00 00 00 00 d0 09  ....:...........
[11718.557150] netdemo packet: 34 06 99 75 90 8e ff 02 00 00 00 00 00 00 00 00  4..u............
[11718.557153] netdemo packet: 00 00 00 00 00 02 85 00 4f 23 00 00 00 00        ........O#....
[11721.161082] netdemo: packet received in hard_start_xmit, len=107
[11721.161110] netdemo packet: 33 33 00 00 00 fb 5a 83 43 3b e9 53 86 dd 60 02  33....Z.C;.S..`.
[11721.161117] netdemo packet: 48 e3 00 35 11 ff fe 80 00 00 00 00 00 00 d0 09  H..5............
[11721.161122] netdemo packet: 34 06 99 75 90 8e ff 02 00 00 00 00 00 00 00 00  4..u............
[11721.161126] netdemo packet: 00 00 00 00 00 fb 14 e9 14 e9 00 35 a7 a4 00 00  ...........5....
[11721.161129] netdemo packet: 00 00 00 02 00 00 00 00 00 00 05 5f 69 70 70 73  ..........._ipps
[11721.161133] netdemo packet: 04 5f 74 63 70 05 6c 6f 63 61 6c 00 00 0c 00 01  ._tcp.local.....
[11721.161136] netdemo packet: 04 5f 69 70 70 c0 12 00 0c 00 01                 ._ipp......
[11721.898179] netdemo: packet received in hard_start_xmit, len=344
[11721.898231] netdemo packet: ff ff ff ff ff ff 5a 83 43 3b e9 53 08 00 45 00  ......Z.C;.S..E.
[11721.898234] netdemo packet: 01 4a 00 00 40 00 40 11 39 a4 00 00 00 00 ff ff  .J..@.@.9.......
[11721.898236] netdemo packet: ff ff 00 44 00 43 01 36 54 25 01 01 06 00 a6 d4  ...D.C.6T%......
[11721.898238] netdemo packet: 96 1a 00 1f 00 00 00 00 00 00 00 00 00 00 00 00  ................
[11721.898239] netdemo packet: 00 00 00 00 00 00 5a 83 43 3b e9 53 00 00 00 00  ......Z.C;.S....
[11721.898240] netdemo packet: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
[11721.898242] netdemo packet: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
[11721.898243] netdemo packet: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
[11721.898244] netdemo packet: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
[11721.898246] netdemo packet: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
[11721.898247] netdemo packet: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
[11721.898248] netdemo packet: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
[11721.898249] netdemo packet: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
[11721.898251] netdemo packet: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
[11721.898252] netdemo packet: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
[11721.898253] netdemo packet: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
[11721.898255] netdemo packet: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
[11721.898256] netdemo packet: 00 00 00 00 00 00 63 82 53 63 35 01 01 3d 07 01  ......c.Sc5..=..
[11721.898257] netdemo packet: 5a 83 43 3b e9 53 37 11 01 02 06 0c 0f 1a 1c 79  Z.C;.S7........y
[11721.898269] netdemo packet: 03 21 28 29 2a 77 f9 fc 11 39 02 02 40 0c 18 75  .!()*w...9..@..u
[11721.898270] netdemo packet: 62 75 6e 74 75 2d 67 6e 75 2d 6c 69 6e 75 78 2d  buntu-gnu-linux-
[11721.898271] netdemo packet: 32 34 2d 30 34 2d 33 ff                          24-04-3.
parallels@ubuntu-gnu-linux-24-04-3:~/drivers/practice/5$ sudo rmmod net_driver
parallels@ubuntu-gnu-linux-24-04-3:~/drivers/practice/5$ ip link show | grep netdemo
parallels@ubuntu-gnu-linux-24-04-3:~/drivers/practice/5$ sudo dmesg | tail -20
[11721.898239] netdemo packet: 00 00 00 00 00 00 5a 83 43 3b e9 53 00 00 00 00  ......Z.C;.S....
[11721.898240] netdemo packet: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
[11721.898242] netdemo packet: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
[11721.898243] netdemo packet: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
[11721.898244] netdemo packet: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
[11721.898246] netdemo packet: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
[11721.898247] netdemo packet: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
[11721.898248] netdemo packet: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
[11721.898249] netdemo packet: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
[11721.898251] netdemo packet: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
[11721.898252] netdemo packet: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
[11721.898253] netdemo packet: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
[11721.898255] netdemo packet: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
[11721.898256] netdemo packet: 00 00 00 00 00 00 63 82 53 63 35 01 01 3d 07 01  ......c.Sc5..=..
[11721.898257] netdemo packet: 5a 83 43 3b e9 53 37 11 01 02 06 0c 0f 1a 1c 79  Z.C;.S7........y
[11721.898269] netdemo packet: 03 21 28 29 2a 77 f9 fc 11 39 02 02 40 0c 18 75  .!()*w...9..@..u
[11721.898270] netdemo packet: 62 75 6e 74 75 2d 67 6e 75 2d 6c 69 6e 75 78 2d  buntu-gnu-linux-
[11721.898271] netdemo packet: 32 34 2d 30 34 2d 33 ff                          24-04-3.
[11731.656269] netdemo: interface stopped
[11731.662984] netdemo: module unloaded
```