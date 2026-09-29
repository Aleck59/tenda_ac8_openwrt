# OpenWrt для Tenda AC8 v1

Сборка **OpenWrt 19.07 (Linux 4.14)** для роутера Tenda AC8 v1 на SoC
Realtek RTL8197FH-VG через GitHub Actions.

> **Статус: экспериментальный.** В официальной OpenWrt нет поддержки
> RTL8197F. Прежде чем что-то шить, прочитайте
> [docs/flashing.md](docs/flashing.md) и сделайте полный дамп флеша
> программатором.

## Почему 19.07

До сборки `ac8-20260928-26` порт был на OpenWrt 24.10 / Linux 6.6
(порт [tobyw121/Openwrt_RTL](https://github.com/tobyw121/Openwrt_RTL)).
Загрузка и LAN там заработали, но Ethernet был нестабилен, а к точке
доступа Wi-Fi клиенты не подключались (четырёхстороннее рукопожатие WPA2
через hostapd и слой cfg80211 вендорского драйвера не завершалось).

Почти все рабочие прошивки для RTL8197F сделаны на старых ядрах вокруг
Realtek SDK: форк OpenWrt 18.06 для Tenda AC10U и порт OpenWrt 19.07 для
Alcatel LINKHUB HH71VM (тот же RTL8197F и тот же RTL8812F на PCIe). Эта
сборка переходит на их путь:

| Узел | Как сделано | Откуда |
|---|---|---|
| Платформа, ядро 4.14, SPI-флеш | цель `rtkmipsel` | порт [HH71VM](https://github.com/sowarden/hh71vm-openwrt) |
| Ethernet | драйвер SDK `rtknet` (`rtl819x`) и драйвер RTL8367RB из SDK Realtek (`rtl8367r/`, API `rtl8367b_*`) — **тот же код, что в стоковой прошивке AC8**: сброс чипа, `rtk_switch_init`, EXT1 RGMII 1000/full с задержками tx 0 / rx 5, светодиоды, пороги, SSC; WAN — порт 4 коммутатора | `rtl8367r/` из форка [AC10U](https://github.com/vladisslav2011/openwrt-AC10) |
| Wi-Fi | вендорский `rtl8192cd` без cfg80211 и без hostapd: точку доступа настраивает netifd через `iwpriv set_mib`, WPA2-PSK считает сам драйвер (как в стоке) | обработчик netifd порта HH71VM |
| Калибровка радио | берётся из данных платы Tenda во флеше (`0x1c000`): таблицы мощности, `xcap`, `ther`, TSSI — как в стоке | эта сборка |

Прежняя сборка 24.10 осталась в истории репозитория (коммит `0f064be`).

## Железо

| Узел | Микросхема | Примечание |
|---|---|---|
| SoC | Realtek RTL8197FH-VG, MIPS 24Kc 1 ГГц | встроенное радио 2,4 ГГц 802.11n 2×2 |
| 5 ГГц | Realtek RTL8812FR (PCIe) | 802.11ac 2×2 |
| Коммутатор | Realtek RTL8367RB-VB | 1 WAN + 3 LAN, гигабит, SMI на GPIO H0/G7 |
| Флеш | SPI NOR 4 МБ | для OpenWrt нужна замена на 8/16 МБ |
| ОЗУ | 64 МБ | |
| Сток | eCos, образ `cs6c` по адресу `0x20000` | загрузчик Realtek в `0x0–0x20000`, данные платы в `0x1c000` |

## Что работает

Сборка 19.07 на железе ещё не запускалась. Ожидается:

| Функция | Как устроено |
|---|---|
| Загрузка | загрузчик AC8 копирует образ `cs6c` с `0x20000` в `0x80a00000`; загрузчик LZMA останавливает watchdog и распаковывает ядро; консоль UART 115200 |
| Флеш | разделы `boot` (только чтение), `firmware`, `tenda_config` (только чтение); `firmware` делится на `kernel`, `rootfs` (SquashFS) и `rootfs_data` (JFFS2: настройки и пакеты) |
| Ethernet | `eth0` — LAN (порты 0–3), `eth1` — WAN (порт 4). Роутер — `192.168.1.1/24`, DHCP-сервер на LAN. MAC-адреса — из данных платы (`et0macaddr`) |
| Wi-Fi | `wlan0` — 2,4 ГГц (SSID `Openwrt`), `wlan1` — 5 ГГц (SSID `Openwrt-5G`, канал 36, VHT80); WPA2-PSK, пароль `12345678`. Видны в LuCI (**Сеть → Беспроводные сети**). MAC-адреса — `wl1_hwaddr`/`wl0_hwaddr` |
| LuCI | 19.07, тема bootstrap, русский язык |
| Светодиоды, кнопка | SYS — E3, WLAN — E1, кнопка WPS/RST — E4 (короткое нажатие — перезагрузка, от 5 с — сброс настроек); светодиоды портов ведёт RTL8367RB |
| Обновление | `sysupgrade`/LuCI, с сохранением настроек |

## Профили и объём флеша

| Образ | Флеш | Раздел `firmware` |
|---|---|---|
| `*-tenda_ac8-v1-8m-*` | 8 МБ | `0x020000–0x7e0000`, 7,75 МБ |
| `*-tenda_ac8-v1-16m-*` | 16 МБ | `0x020000–0xfe0000`, 15,75 МБ |

Ядро одно, образы различаются только таблицей разделов в командной строке
загрузчика LZMA. На стоковую микросхему 4 МБ прошивка не помещается.

## Файлы в Releases

| Файл | Назначение |
|---|---|
| `*-{8m,16m}-fullflash.bin` | полный образ микросхемы **с загрузчиком** из `boot/tenda_ac8-v1-boot.bin` (только для роутера, с которого он снят); `flashrom -p ch341a_spi -w` |
| `*-{8m,16m}-squashfs-flash.bin` | раздел `firmware` с `0x20000`: для загрузчика (TFTP + `FLW`) или программатора |
| `*-{8m,16m}-squashfs-sysupgrade.bin` | то же с метаданными: обновление из LuCI или `sysupgrade` |

Как шить — [docs/flashing.md](docs/flashing.md).

## Сборка в GitHub Actions

**Actions → Build OpenWrt 19.07 (Tenda AC8 v1) → Run workflow**
(`extra_packages` — дополнительные пакеты, `release` — выложить в
Releases). Сборка запускается и сама при push в `main`, если меняются
`target/`, `patches/`, `configs/`, `base.env` или скрипты. OpenWrt 19.07
нужен Python 2, поэтому раннер — Ubuntu 22.04. Первая сборка — около часа
(тулчейн), дальше тулчейн берётся из кеша.

## Локальная сборка

Нужен Ubuntu 22.04 (или chroot/контейнер с ним) с
[зависимостями сборки OpenWrt](https://openwrt.org/docs/guide-developer/build-system/install-buildsystem)
и `python2`.

```sh
./scripts/prepare.sh openwrt       # исходники + цель HH71VM + rtl8367r + файлы AC8
./scripts/configure.sh openwrt
cd openwrt
make download -j8
make -j"$(nproc)"
ls bin/targets/rtkmipsel/rtl8197f/
```

## Устройство репозитория

```
base.env                   закреплённые ревизии: OpenWrt 19.07, feeds, HH71VM, AC10U
configs/tenda_ac8.config   затравка .config (цель, профиль, пакеты)
target/linux/rtkmipsel/    файлы AC8 поверх цели HH71VM:
  Makefile, rtl8197f/        цель, профиль tenda_ac8, конфиг ядра 4.14
  image/Makefile             образы cs6c для 8/16 МБ
  files/.../mach-tenda-ac8.c светодиоды и кнопка
  files/.../mtdsplit_rtl8197f.c, patches-4.14/410-*
                             разбиение firmware на kernel/rootfs/rootfs_data
  base-files/                сеть, Wi-Fi (netifd + калибровка), sysupgrade, LED
patches/                   изменения файлов HH71VM и OpenWrt:
  0001  машина Tenda AC8 вместо HH71VM
  0002  RTL8367RB: SMI на H0/G7, без GPIO сброса, пады и задержки P0 как в стоке
  0003  загрузчик LZMA останавливает watchdog
  0004  кольца rtl8192cd под 64 МБ (TX 512, RX 512/256)
  0005  сборка .ipk без гонки при make -j (из HH71VM)
scripts/prepare.sh         собирает дерево из всех источников
scripts/configure.sh       пишет .config и проверяет, что ничего не выпало
scripts/make-fullflash.py  полный образ микросхемы для программатора
boot/                      ваш загрузчик для fullflash-образов (см. boot/README.md)
docs/flashing.md           UART, прошивка, проверка, откат
docs/release-notes.md      текст, который публикуется с каждым релизом
```

## Источники

- OpenWrt 19.07: <https://github.com/openwrt/openwrt> (ветка `openwrt-19.07`);
- порт Alcatel LINKHUB HH71VM (OpenWrt 19.07, Linux 4.14, RTL8197F +
  RTL8812FE): <https://github.com/sowarden/hh71vm-openwrt>, GPL-2.0;
- форк OpenWrt 18.06 для Tenda AC10U (RTL8197F + RTL8367RB):
  <https://github.com/vladisslav2011/openwrt-AC10>;
- обсуждение RTL8197FH и Tenda AC8 на форуме OpenWrt:
  <https://forum.openwrt.org/t/openwrt-on-rtl8197fh/89446>.

Лицензия файлов этого репозитория — GPL-2.0, как у OpenWrt.
