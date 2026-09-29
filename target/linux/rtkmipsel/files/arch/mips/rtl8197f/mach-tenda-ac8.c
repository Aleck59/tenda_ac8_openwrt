/*
 * Tenda AC8 v1 (Realtek RTL8197FH-VG + RTL8812FR + RTL8367RB-VB)
 *
 * GPIOs from the code of the stock eCos firmware of the AC8 v1: its GPIO
 * init sets up only E3 (output, the system LED, low = on) and E4 (input,
 * the WPS/RST button, low = pressed).  The WLAN LED on E1 was verified on
 * the board (rightmost LED).  The port LEDs belong to the RTL8367RB, the
 * RTL8367 SMI pins (H0 = MDC, G7 = MDIO) to the Ethernet driver
 * (CONFIG_RTL_MDC_H0_MDIO_G7).
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/leds.h>

#include "bspchip.h"
#include "machtypes.h"
#include "dev_leds_gpio.h"
#include "dev-gpio-buttons.h"

#define AC8_KEYS_POLL_INTERVAL		20	/* msecs */
#define AC8_KEYS_DEBOUNCE_INTERVAL	(3 * AC8_KEYS_POLL_INTERVAL)

extern void rtl819x_gpio_pin_enable(u32 pin);

static struct gpio_led ac8_leds_gpio[] __initdata = {
	{
		.name		= "blue:sys",
		.gpio		= BSP_GPIO_PIN_E3,
		.active_low	= 1,
	}, {
		.name		= "blue:wlan",
		.gpio		= BSP_GPIO_PIN_E1,
		.active_low	= 1,
	},
};

static struct gpio_keys_button ac8_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = AC8_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= BSP_GPIO_PIN_E4,
		.active_low	= 1,
	},
};

static void __init tenda_ac8_setup(void)
{
	int i;

	/* Pin mux: GPIO function, the values of the stock pin mux helper. */
	for (i = 0; i < ARRAY_SIZE(ac8_leds_gpio); i++)
		rtl819x_gpio_pin_enable(ac8_leds_gpio[i].gpio);
	for (i = 0; i < ARRAY_SIZE(ac8_gpio_keys); i++)
		rtl819x_gpio_pin_enable(ac8_gpio_keys[i].gpio);

	rtl819x_register_leds_gpio(-1, ARRAY_SIZE(ac8_leds_gpio),
				   ac8_leds_gpio);
	rtl819x_add_device_gpio_buttons(-1, AC8_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(ac8_gpio_keys),
					ac8_gpio_keys);
}

MIPS_MACHINE(RTL8197_MACH_TENDA_AC8, "TENDA-AC8", "Tenda AC8 v1",
	     tenda_ac8_setup);
