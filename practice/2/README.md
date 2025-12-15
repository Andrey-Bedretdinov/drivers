
## Сборка модуля

Собираем модуль с помощью команды `make`. В выводе видно, что сборка прошла успешно, хотя компилятор отличается от того, что использовался для сборки ядра.

```shell
parallels@ubuntu-gnu-linux-24-04-3:~/drivers/practice/2$ make
make -C /lib/modules/6.14.0-36-generic/build M=/home/parallels/drivers/practice/2 modules
make[1]: вход в каталог «/usr/src/linux-headers-6.14.0-36-generic»
make[2]: вход в каталог «/home/parallels/drivers/practice/2»
warning: the compiler differs from the one used to build the kernel
The kernel was built by: aarch64-linux-gnu-gcc-13 (Ubuntu 13.3.0-6ubuntu2~24.04) 13.3.0
You are using:           gcc-13 (Ubuntu 13.3.0-6ubuntu2~24.04) 13.3.0
CC [M]  simple.o
MODPOST Module.symvers
CC [M]  simple.mod.o
CC [M]  .module-common.o
LD [M]  simple.ko
BTF [M] simple.ko
Skipping BTF generation for simple.ko due to unavailability of vmlinux
make[2]: выход из каталога «/home/parallels/drivers/practice/2»
make[1]: выход из каталога «/usr/src/linux-headers-6.14.0-36-generic»
```

---

## Загрузка модуля

Загружаем модуль в ядро с помощью `insmod`. Ошибок нет, значит модуль успешно загружен.

```shell
parallels@ubuntu-gnu-linux-24-04-3:~/drivers/practice/2$ sudo insmod simple.ko
```

---

## Проверка через lsmod и dmesg

Проверяем, что модуль действительно загружен с помощью `lsmod`. Затем смотрим последние сообщения ядра через `dmesg`, чтобы убедиться, что модуль работает и выводит свои сообщения.

```shell
parallels@ubuntu-gnu-linux-24-04-3:~/drivers/practice/2$ lsmod | grep simple
simple                 12288  0
```


```shell
parallels@ubuntu-gnu-linux-24-04-3:~/drivers/practice/2$ dmesg | tail -20
dmesg: чтение буфера ядра завершилось неудачно: Операция не позволена
parallels@ubuntu-gnu-linux-24-04-3:~/drivers/practice/2$ sudo dmesg | tail -20
[  622.181890] audit: type=1400 audit(1765057486.605:175): apparmor="STATUS" operation="profile_replace" profile="unconfined" name="snap.firefox.firefox" pid=5369 comm="apparmor_parser"
[  622.608793] audit: type=1400 audit(1765057487.031:176): apparmor="STATUS" operation="profile_replace" profile="unconfined" name="snap.firefox.geckodriver" pid=5373 comm="apparmor_parser"
[  622.703272] audit: type=1400 audit(1765057487.126:177): apparmor="STATUS" operation="profile_replace" profile="unconfined" name="snap.firefox.hook.configure" pid=5374 comm="apparmor_parser"
[  622.766271] audit: type=1400 audit(1765057487.189:178): apparmor="STATUS" operation="profile_replace" profile="unconfined" name="snap.firefox.hook.disconnect-plug-host-hunspell" pid=5375 comm="apparmor_parser"
[  629.084905] loop8: detected capacity change from 0 to 141184
[  642.603685] loop0: detected capacity change from 0 to 479584
[  642.793003] kauditd_printk_skb: 6 callbacks suppressed
[  642.793008] audit: type=1400 audit(1765057507.216:185): apparmor="STATUS" operation="profile_replace" info="same as current profile, skipping" profile="unconfined" name="/snap/snapd/25585/usr/lib/snapd/snap-confine" pid=5972 comm="apparmor_parser"
[  642.793011] audit: type=1400 audit(1765057507.216:186): apparmor="STATUS" operation="profile_replace" info="same as current profile, skipping" profile="unconfined" name="/snap/snapd/25585/usr/lib/snapd/snap-confine//mount-namespace-capture-helper" pid=5972 comm="apparmor_parser"
[  643.218666] audit: type=1400 audit(1765057507.641:187): apparmor="STATUS" operation="profile_replace" profile="unconfined" name="snap-update-ns.firefox" pid=5974 comm="apparmor_parser"
[  643.698195] audit: type=1400 audit(1765057508.121:188): apparmor="STATUS" operation="profile_replace" profile="unconfined" name="snap.firefox.firefox" pid=5978 comm="apparmor_parser"
[  644.205191] audit: type=1400 audit(1765057508.628:189): apparmor="STATUS" operation="profile_replace" profile="unconfined" name="snap.firefox.geckodriver" pid=5979 comm="apparmor_parser"
[  644.319799] audit: type=1400 audit(1765057508.742:190): apparmor="STATUS" operation="profile_replace" profile="unconfined" name="snap.firefox.hook.configure" pid=5983 comm="apparmor_parser"
[  644.394257] audit: type=1400 audit(1765057508.817:191): apparmor="STATUS" operation="profile_replace" profile="unconfined" name="snap.firefox.hook.disconnect-plug-host-hunspell" pid=5984 comm="apparmor_parser"
[  644.470310] audit: type=1400 audit(1765057508.893:192): apparmor="STATUS" operation="profile_replace" profile="unconfined" name="snap.firefox.hook.install" pid=5985 comm="apparmor_parser"
[  644.549400] audit: type=1400 audit(1765057508.972:193): apparmor="STATUS" operation="profile_replace" profile="unconfined" name="snap.firefox.hook.post-refresh" pid=5986 comm="apparmor_parser"
[  644.670288] audit: type=1400 audit(1765057509.093:194): apparmor="STATUS" operation="profile_replace" info="same as current profile, skipping" profile="unconfined" name="/snap/snapd/25585/usr/lib/snapd/snap-confine" pid=6005 comm="apparmor_parser"
[ 3282.396894] simple: loading out-of-tree module taints kernel.
[ 3282.396930] simple: module verification failed: signature and/or required key missing - tainting kernel
[ 3282.400583] Hello from the mai 307 team
```

---

## Удаление модуля

Удаляем модуль из ядра с помощью `rmmod` и проверяем, что он больше не загружен.

```shell
parallels@ubuntu-gnu-linux-24-04-3:~/drivers/practice/2$ sudo rmmod simple
parallels@ubuntu-gnu-linux-24-04-3:~/drivers/practice/2$ lsmod | grep simple
```


