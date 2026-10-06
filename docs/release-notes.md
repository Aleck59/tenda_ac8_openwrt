OpenWrt для **Tenda AC8 v1** (RTL8197FH + RTL8812FR + RTL8367RB, 64 МиБ ОЗУ) с
флешкой, **заменённой на 8 МиБ**. Версия `@VERSION@`.

**Файлы**

* `*-squashfs-sysupgrade.bin` — основной образ: для программатора соберите полный
  образ из своего дампа (`scripts/mkflash.py`), из OpenWrt обновляйтесь через LuCI или
  `sysupgrade`.
* `*-initramfs-nfjrom.bin` — запуск из ОЗУ через TFTP загрузчика (имя файла `nfjrom`),
  для первой установки без программатора и восстановления.

**Что внутри:** OpenWrt (Linux 6.18), LuCI на русском с темой Footstrap, HTTPS,
PPPoE, Wi‑Fi 2.4/5 ГГц с WPA2/WPA3, калибровка и MAC-адреса из заводского блока
роутера, zram-swap.

Инструкция: [docs/install.md](https://github.com/@REPO@/blob/@VERSION@/docs/install.md). Перед установкой
сохраните полный дамп флешки — в нём уникальная калибровка Wi‑Fi.
