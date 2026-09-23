**Экспериментальная сборка**: OpenWrt 24.10, Linux 6.6, порт RTL8197F
[tobyw121/Openwrt_RTL](https://github.com/tobyw121/Openwrt_RTL). На Tenda AC8
её ещё никто не запускал. Флеш смонтирован только для чтения, настройки
не сохраняются. Прочитайте [инструкцию](https://github.com/Aleck59/tenda_ac8_openwrt/blob/main/docs/flashing.md)
и снимите полный дамп флеша, прежде чем что-то записывать.

| Файл | Для чего |
|---|---|
| `*-tenda_ac8-v1-initramfs-kernel.bin` | загрузка из RAM на стоковой микросхеме 4 МБ (TFTP в `0x80a00000`, `J 80a00000`), флеш не меняется |
| `*-tenda_ac8-v1-{8m,16m}-initramfs-kernel.bin` | то же для микросхем 8/16 МБ |
| `*-{8m,16m}-squashfs-flash.bin` | прошивка для записи с адреса `0x20000` (из загрузчика командой `FLW`) |
| `*-{8m,16m}-fullflash.bin` | полный образ микросхемы **с загрузчиком** из `boot/` репозитория — только для роутера, с которого снят этот загрузчик (есть, если `boot/tenda_ac8-v1-boot.bin` добавлен) |
| `*-{8m,16m}-programmer-firmware-region.bin` + `.layout` | образ размером с микросхему для программатора, **без загрузчика** |
| `make-fullflash.py` | полный образ микросхемы из вашего дампа (с вашим загрузчиком) |
| `sha256sums` | контрольные суммы |

Для стоковой микросхемы 4 МБ прошивки во флеш нет: она не помещается.

**Образ для программатора** пишите только по регионам, иначе будет стёрт
загрузчик:

```sh
flashrom -p ch341a_spi -l openwrt-realtek-rtl8197f-tenda_ac8-v1-16m-programmer-firmware-region.bin.layout \
    -i firmware -w openwrt-realtek-rtl8197f-tenda_ac8-v1-16m-programmer-firmware-region.bin
```

На микросхеме уже должен быть загрузчик: стоковая, или новая с записанным
вашим дампом. Чистую новую микросхему шейте целиком образом
`*-fullflash.bin` (`flashrom -p ch341a_spi -w ...-fullflash.bin`) или
полным образом из `make-fullflash.py --backup ваш_дамп.bin ...`.
