- Добавить драйвер из предыдущей разработки (net + pci) в ядро, чтобы он был виден в make menuconfig 
- Создать патч

---

### 1. Скачал исходники ядра

Взял дерево Торвальдса:

git clone https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git
cd linux

Выбрал версию:

git checkout v6.8

(версия может быть любой, просто нужна стабильная точка).

⸻

### 2. Создал директорию для драйвера

Добавил собственную папку внутри ethernet-драйверов:

drivers/net/ethernet/mydriver/

Положил туда файл:

pci_net_driver.c


⸻

### 3. Добавил Makefile для сборки драйвера

В папку mydriver положил:

obj-$(CONFIG_MYDRIVER) += pci_net_driver.o


⸻

### 4. Создал Kconfig для отображения пункта в menuconfig

drivers/net/ethernet/mydriver/Kconfig

Содержимое:

config MYDRIVER
        tristate "My PCI Network Driver"
        depends on PCI
        help
          Простой учебный PCI сетевой драйвер.
          Реализует probe/remove, чтение MAC-адреса
          и заглушку передачи пакетов.


⸻

### 5. Подключил пункт в общий Kconfig

В конец файла:

drivers/net/ethernet/Kconfig

добавил:

source "drivers/net/ethernet/mydriver/Kconfig"

Теперь драйвер появился в make menuconfig.

⸻

### 6. Добавил модуль в общий Makefile ethernet-драйверов

В файл:

drivers/net/ethernet/Makefile

добавлено:

obj-$(CONFIG_MYDRIVER) += mydriver/


⸻

### 7. Проверил, что опция появилась в menuconfig

make menuconfig

Пункт:

Device Drivers → Network device support → Ethernet → My PCI Network Driver

стал виден и собирается как M.

⸻

### 8. Подготовил git к созданию патча

Добавил новые файлы:

git add drivers/net/ethernet/mydriver/
git add drivers/net/ethernet/Kconfig
git add drivers/net/ethernet/Makefile

Закоммитил:

git commit -s -m "add PCI network demo driver

This patch adds a simple educational PCI network driver.
Implements probe/remove, MAC read logic and dummy xmit."


⸻

### 9. Создал патч

git format-patch -1

Получился файл вида:

0001-add-PCI-network-demo-driver.patch


⸻

### 10. Прогнал checkpatch.pl

./scripts/checkpatch.pl --strict 0001-add-PCI-network-demo-driver.patch

Ошибок — 0
