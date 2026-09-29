#
# sysupgrade for the Tenda AC8 v1: the image (cs6c header, loader + kernel,
# SquashFS, JFFS2 marker) is written to the "firmware" partition at flash
# 0x20000 as it is; the rtl8197f-fw parser finds rootfs and rootfs_data in it.
#

PART_NAME=firmware
REQUIRE_IMAGE_METADATA=1

platform_check_image() {
	[ "$#" -gt 1 ] && return 1

	case "$(board_name)" in
	tenda,ac8-v1*)
		# "cs6c": the header the Realtek boot loader checks.
		[ "$(get_magic_long "$1")" = "63733663" ] || {
			echo "Invalid image type: no cs6c header."
			return 1
		}
		return 0
		;;
	esac

	echo "Sysupgrade is not supported on this board."
	return 1
}

platform_do_upgrade() {
	default_do_upgrade "$1"
}
