#
# Target hook of package/base-files (included before BuildPackage).
#
# /etc/opkg/distfeeds.conf: rtkmipsel/rtl8197f is not an upstream target, so
# there is no "core" feed with its kernel modules.  The other packages come
# from the upstream 19.07.10 release: the architecture is plain mipsel_24kc
# and the base package versions of the pinned tree are the ones 19.07.10 was
# built from (libubox, libubus, libuci, busybox, opkg; checked by the HH71VM
# port against packages/mipsel_24kc/base/Packages.gz of 19.07.10).
#
# This is free software, licensed under the GNU General Public License v2.
#

AC8_PKG_RELEASE:=https://downloads.openwrt.org/releases/19.07.10

# 1: destination file (macro called from package/base-files/Makefile)
define FeedSourcesAppend
( \
  echo '# Packages from the upstream OpenWrt 19.07.10 release (mipsel_24kc).'; \
  echo '# Kernel modules for this target exist only in the image: add them by'; \
  echo '# building a new image, not with opkg.'; \
  echo 'src/gz %d_base $(AC8_PKG_RELEASE)/packages/%A/base'; \
  echo 'src/gz %d_luci $(AC8_PKG_RELEASE)/packages/%A/luci'; \
  echo 'src/gz %d_packages $(AC8_PKG_RELEASE)/packages/%A/packages'; \
  echo 'src/gz %d_routing $(AC8_PKG_RELEASE)/packages/%A/routing'; \
  echo 'src/gz %d_telephony $(AC8_PKG_RELEASE)/packages/%A/telephony'; \
) >> $(1)
endef
