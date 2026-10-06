# OpenWrt для Tenda AC8 v1 (8 МиБ флеш)

Сборка OpenWrt (Linux 6.18) для роутера **Tenda AC8 v1** с флешкой,
заменённой с 2 на 8 МиБ:

* SoC **Realtek RTL8197FH** (MIPS 24Kc, 1 ГГц), 64 МиБ DDR2;
* Wi‑Fi 2.4 ГГц — встроенный RTL8197F (2T2R n), 5 ГГц — **RTL8812FR** на PCIe (2T2R ac);
* гигабитный коммутатор **RTL8367RB-VB**: 3 × LAN + WAN.

## Что есть в прошивке

| | |
|---|---|
| Сеть | DSA-коммутатор: `lan1`–`lan3`, `wan`; MAC-адреса из заводского блока, как в стоке |
| Wi‑Fi | оба диапазона (вендорный драйвер `rtl8192cd` через cfg80211/nl80211, hostapd); **WPA2/WPA3** (SAE + 802.11w); калибровка мощности/TSSI/кварца из заводского NVRAM каждого роутера |
| WAN | DHCP, статика, **PPPoE**; IPv6 |
| Интерфейс | LuCI на русском, тема **[Footstrap](https://github.com/VizzleTF/luci-theme-footstrap)** (по умолчанию), HTTPS |
| Прочее | программный flow offloading, zram-swap, светодиод статуса, кнопка WPS/RST (перезагрузка / сброс) |

Wi‑Fi включён сразу: `OpenWrt-AC8-XXXXXX` и `OpenWrt-AC8-XXXXXX-5G`, пароль —
**WPS PIN роутера** из заводского блока. Подробности — в
[docs/install.md](docs/install.md#3-после-установки).

## Установка

Коротко: сохраните полный дамп флешки, соберите из него и `sysupgrade.bin`
полный образ скриптом `scripts/mkflash.py` и запишите программатором. Без
программатора — через TFTP загрузчика (`nfjrom`) и sysupgrade.

**[docs/install.md](docs/install.md)** — пошаговая инструкция.

Готовые образы — в [Releases](../../releases) (собираются GitHub Actions).

## Документация

* [docs/install.md](docs/install.md) — установка, обновление, восстановление;
* [docs/hardware.md](docs/hardware.md) — железо, разметка флеш, загрузчик,
  формат заводского NVRAM и калибровки Wi‑Fi;
* [docs/building.md](docs/building.md) — сборка, свои пакеты, CI/CD и релизы.

## Состояние

Прошивка собирается целиком (ядро, драйверы, образы, проверка размера), формат
образа сверен со стоковым заголовком загрузчика байт-в-байт. Платформенная
часть (ядро 6.18 для RTL8197F, Ethernet, PCIe, DSA RTL8367RB-VB, драйвер
RTL8197F Wi‑Fi) проверена на железе в проекте ipTIME A2004MU; специфичное для
AC8 (раскладка GPIO/портов, двухдиапазонный режим драйвера с RTL8812F,
калибровка из NVRAM Tenda, загрузка с `cs6c`) взято из разбора стоковой
прошивки и **на AC8 ещё не проверялось** — первый запуск делайте с
программатором под рукой и подключённым UART (38400 8N1). Отчёты о проблемах —
в Issues, с логом консоли.

## Происхождение и лицензии

* Платформа RTL8197F для OpenWrt/Linux 6.18 — по
  [Putpocket/iptime-a2004mu-openwrt](https://github.com/Putpocket/iptime-a2004mu-openwrt)
  (GPL-2.0), доработана для AC8: DTS, раскладка SMI/портов, ревизия RGMII,
  парсер разделов `cvimg`, двухступенчатый загрузчик ядра.
* Драйвер Wi‑Fi `rtl8192cd` — исходники Realtek из GPL-релиза Cudy GP3000,
  порт на 6.18 по A2004MU; добавлены RTL8812F на шине PCI, калибровка из NVRAM
  Tenda, совместимость DMA-API, уменьшенные кольца для 64 МиБ.
* Тема Footstrap — © Ivan Kvashonkin, Apache-2.0, подключается фидом.
* Остальное — GPL-2.0 ([LICENSE](LICENSE)), как и OpenWrt.
