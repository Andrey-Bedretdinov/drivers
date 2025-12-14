- Добавить драйвер из предыдущей разработки (net + pci) в ядро, чтобы он был виден в make menuconfig 
- Создать патч

---

### Скачал исходники ядра

```shell
git clone https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git
cd linux
```
Выбрал версию:
```shell
git checkout v6.7
```

---

### Создал директорию для драйвера

Добавил собственную папку внутри ethernet-драйверов:
```shell
drivers/net/ethernet/mydriver/
```
Положил туда файл:
```shell
pci_net_driver.c
```

---

### Добавил Makefile для сборки драйвера

В папку mydriver положил:
```shell
obj-$(CONFIG_MYDRIVER) += pci_net_driver.o
```

---

### Создал Kconfig для отображения пункта в menuconfig
```shell
drivers/net/ethernet/mydriver/Kconfig
```
Содержимое:
```shell
config MYDRIVER
        tristate "My PCI Network Driver"
        depends on PCI
        help
          Простой учебный PCI сетевой драйвер.
          Реализует probe/remove, чтение MAC-адреса
          и заглушку передачи пакетов.
```

---

### Подключил пункт в общий Kconfig

В конец файла:
```shell
drivers/net/ethernet/Kconfig
```
добавил:
```shell
source "drivers/net/ethernet/mydriver/Kconfig"
```
Теперь драйвер появился в make menuconfig.

---

### Добавил модуль в общий Makefile ethernet-драйверов

В файл:
```shell
drivers/net/ethernet/Makefile
```
добавлено:
```shell
obj-$(CONFIG_MYDRIVER) += mydriver/
```

---

### Проверил, что опция появилась в menuconfig
```shell
make menuconfig
```
Пункт:
```shell
Device Drivers → Network device support → Ethernet → My PCI Network Driver
```
стал виден.

```shell
 .config - Linux/x86 6.6.0 Kernel Configuration
 > Device Drivers > Network device support > Ethernet driver support ──────────────────────────────────────────────────────────────────────────────────────────────────
  ┌──────────────────────────────────────────────────────────────────── Ethernet driver support ────────────────────────────────────────────────────────────────────┐
  │  Arrow keys navigate the menu.  <Enter> selects submenus ---> (or empty submenus ----).  Highlighted letters are hotkeys.  Pressing <Y> includes, <N> excludes, │  
  │  <M> modularizes features.  Press <Esc><Esc> to exit, <?> for Help, </> for Search.  Legend: [*] built-in  [ ] excluded  <M> module  < > module capable         │  
  │                                                                                                                                                                 │  
  │                                                                                                                                                                 │  
  │ ┌───────────────────────────────────────────^(-)──────────────────────────────────────────────────────────────────────────────────────────────────────────────┐ │  
  │ │                                           [ ]       Support for STMMAC Selftests                                                                            │ │  
  │ │                                           <M>       STMMAC Platform bus support                                                                             │ │  
  │ │                                           <M>         Generic driver for DWMAC                                                                              │ │  
  │ │                                           <M>       Intel GMAC support                                                                                      │ │  
  │ │                                           < >       Loongson PCI DWMAC support (NEW)                                                                        │ │  
  │ │                                           <M>       STMMAC PCI bus support                                                                                  │ │  
  │ │                                           [*]   Sun devices                                                                                                 │ │  
  │ │                                           <M>     Sun Happy Meal 10/100baseT support                                                                        │ │  
  │ │                                           <M>     Sun GEM support                                                                                           │ │  
  │ │                                           <M>     Sun Cassini support                                                                                       │ │  
  │ │                                           <M>     Sun Neptune 10Gbit Ethernet support                                                                       │ │  
  │ │                                           [*]   Synopsys devices                                                                                            │ │  
  │ │                                           <M>     Synopsys DWC Enterprise Ethernet (XLGMAC) driver support                                                  │ │  
  │ │                                           <M>       XLGMAC PCI bus support                                                                                  │ │  
  │ │                                           [*]   Tehuti devices                                                                                              │ │  
  │ │                                           <M>     Tehuti Networks 10G Ethernet                                                                              │ │  
  │ │                                           [*]   Texas Instruments (TI) devices                                                                              │ │  
  │ │                                           [ ]     TI CPSW Phy mode Selection (DEPRECATED)                                                                   │ │  
  │ │                                           <M>     TI ThunderLAN support                                                                                     │ │  
  │ │                                           [*]   Vertexcom devices                                                                                           │ │  
  │ │                                           <M>     Vertexcom MSE102x SPI                                                                                     │ │  
  │ │                                           [*]   VIA devices                                                                                                 │ │  
  │ │                                           <M>     VIA Rhine support                                                                                         │ │  
  │ │                                           [*]       Use MMIO instead of PIO                                                                                 │ │  
  │ │                                           <M>     VIA Velocity support                                                                                      │ │  
  │ │                                           [*]   Wangxun devices                                                                                             │ │  
  │ │                                           <M>     Wangxun(R) GbE PCI Express adapters support                                                               │ │  
  │ │                                           <M>     Wangxun(R) 10GbE PCI Express adapters support                                                             │ │  
  │ │                                           [*]   WIZnet devices                                                                                              │ │  
  │ │                                           <M>     WIZnet W5100 Ethernet support                                                                             │ │  
  │ │                                           <M>     WIZnet W5300 Ethernet support                                                                             │ │  
  │ │                                                   WIZnet interface mode (Select interface mode in runtime)  --->                                            │ │  
  │ │                                           <M>     WIZnet W5100/W5200/W5500 Ethernet support for SPI mode                                                    │ │  
  │ │                                           [*]   Xilinx devices                                                                                              │ │  
  │ │                                           <M>     Xilinx 10/100 Ethernet Lite support                                                                       │ │  
  │ │                                           <M>     Xilinx 10/100/1000 AXI Ethernet support                                                                   │ │  
  │ │                                           <M>     Xilinx LL TEMAC (LocalLink Tri-mode Ethernet MAC) driver                                                  │ │  
  │ │                                           [*]   Xircom devices                                                                                              │ │  
  │ │                                           <M>     Xircom 16-bit PCMCIA support                                                                              │ │  
  │ │                                           < >   My PCI Network Driver (NEW)                                                                                 │ │  
  │ └─────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────┘ │  
  ├─────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────┤  
  │                                                    <Select>    < Exit >    < Help >    < Save >    < Load >                                                     │  
  └─────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────┘  
    

```

---

### Подготовил git к созданию патча

Добавил новые файлы:
```shell
git add drivers/net/ethernet/mydriver/
git add drivers/net/ethernet/Kconfig
git add drivers/net/ethernet/Makefile
```
Закоммитил:
```shell
git commit -s -m "add PCI network demo driver
```

---

### 9. Создал патч
```shell
git format-patch -1
```

Получился файл вида:
```shell
0001-add-PCI-network-demo-driver.patch
```

---

### Прогнал checkpatch.pl
```shell
./scripts/checkpatch.pl --strict 0001-add-PCI-network-demo-driver.patch
```
Ошибок — 0
