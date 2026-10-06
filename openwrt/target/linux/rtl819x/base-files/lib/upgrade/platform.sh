# SPDX-License-Identifier: GPL-2.0-only

PART_NAME=firmware
REQUIRE_IMAGE_METADATA=1

platform_check_image() {
	local board magic

	board="$(board_name)"

	case "$board" in
	tenda,ac8-v1)
		# Realtek cvimg boot header "cs6c", burned at flash 0x20000
		magic="$(get_magic_long "$1")"
		[ "$magic" = "63733663" ] || {
			echo "Invalid image: no Realtek cs6c boot header (magic $magic)."
			return 1
		}
		return 0
		;;
	esac

	echo "Sysupgrade is not supported on $board."
	return 1
}

platform_do_upgrade() {
	default_do_upgrade "$1"
}
