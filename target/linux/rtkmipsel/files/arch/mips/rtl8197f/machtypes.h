/*
 * Machine types Realtek RTL8197F
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#ifndef _RTL8197_MACHTYPE_H
#define _RTL8197_MACHTYPE_H

#include <asm/mips_machine.h>

/*
 * The only board of this port is number 0: mips_machine_setup() compares
 * with mips_machtype, which starts at 0, so the board is set up with or
 * without "machtype=" on the command line.
 */
enum rtl8197_mach_type {
	RTL8197_MACH_TENDA_AC8 = 0,	/* Tenda AC8 v1 */
};

#endif /* _RTL8197_MACHTYPE_H */
