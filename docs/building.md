# Сборка

Репозиторий — это **оверлей** поверх OpenWrt, а не полное дерево:

| Путь | Что это |
|---|---|
| `configs/openwrt-base.txt` | репозиторий и коммит OpenWrt, на котором всё проверено |
| `configs/feeds.conf` | фиды, закреплённые на коммитах (packages, luci, тема footstrap) |
| `configs/tenda_ac8.config` | конфигурация прошивки (seed для `make defconfig`) |
| `openwrt/target/linux/rtl819x/` | платформа RTL8197F: патчи ядра 6.18, драйверы Ethernet/PCIe, DTS AC8, рецепт образа, base-files |
| `openwrt/package/kernel/rtl8192cd/` | драйвер Wi‑Fi Realtek (RTL8197F + RTL8812F) |
| `patches/openwrt/` | небольшие правки ядра OpenWrt (wifi-scripts) |
| `boot/` | загрузчик и эталонный заводской блок для образа программатора `*-full-8m.bin` ([boot/README.md](../boot/README.md)) |
| `scripts/` | подготовка дерева, сборка, утилиты для дампа |

## Локально

Нужен Linux с зависимостями OpenWrt (Debian/Ubuntu):

```sh
sudo apt install build-essential clang flex bison g++ gawk gcc-multilib \
  g++-multilib gettext git libncurses-dev libssl-dev python3-setuptools \
  rsync swig unzip zlib1g-dev file wget zstd
```

```sh
./scripts/build.sh          # prepare + download + make, образы в out/ (+ full-8m)
```

`scripts/prepare.sh` скачивает OpenWrt на закреплённом коммите в
`build/openwrt`, копирует оверлей, накладывает патчи, подтягивает фиды
(shallow) и применяет конфигурацию. Повторный запуск безопасен: дерево
сбрасывается к коммиту, `dl/`, `build_dir/` и `staging_dir/` сохраняются.

Первая сборка (тулчейн + ядро + пакеты) занимает 1–2 часа и ~20 ГБ диска.

### Свои пакеты

```sh
OPENWRT_DIR=build/openwrt ./scripts/prepare.sh
make -C build/openwrt menuconfig
SKIP_PREPARE=1 ./scripts/build.sh
```

Либо положите строки `CONFIG_PACKAGE_xxx=y` в файл и передайте его через
`CONFIG_EXTRA=файл ./scripts/build.sh`. Следите за размером: под систему и
пакеты доступно ~7.9 МиБ, проверка размера встроена в сборку образа.

### Быстрая пересборка драйвера Wi‑Fi

```sh
make -C build/openwrt package/kernel/rtl8192cd/{clean,compile} V=s
```

## CI/CD (GitHub Actions)

`.github/workflows/build.yml`:

* **push в `main`, pull request, ручной запуск** — статические проверки
  (shell, python, компиляция DTS) и полная сборка; образы прикладываются к
  запуску как artifact.
* **тег `vX.Y.Z`** или ручной запуск с параметром `release` — то же самое плюс GitHub Release с образами, `sha256sums`,
  манифестом пакетов и `*.buildinfo`. Тег с дефисом (`v1.1.0-rc1`) публикуется
  как pre-release.

Выпуск версии — любым из двух способов:

* **Actions → Build firmware → Run workflow**, поле `release` = `v1.0.0`
  (ветка `main`): workflow соберёт прошивку, создаст тег и релиз;
* или тегом из git:

  ```sh
  git tag -a v1.0.0 -m "Tenda AC8 OpenWrt v1.0.0"
  git push origin v1.0.0
  ```

Версия попадает в прошивку (`CONFIG_VERSION_CODE`, видна в LuCI и
`/etc/openwrt_release`).
