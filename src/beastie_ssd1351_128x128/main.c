
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h> /* bzero */
#include <libgpio.h>
#include <err.h>

#include "lcd.h"

#include "lcd_ssd1351.h"

#include "freebsd-64x64.h"

int
main(int argc, const char *argv[])
{
	int16_t x, y;
	uint32_t c;
	const char *hd = header_data;
	uint8_t col[3];
	struct lcd *lcd;
	uint32_t row_rgb[64];
	struct lcd_ssd1351_cfg cfg;

	/* Configured for the carambola 2 board */
	cfg.gpio_unit = 0;
	cfg.pin_cs = 19;
	cfg.pin_rst = 20;
	cfg.pin_dc = 21;
	cfg.pin_sck = 22;
	cfg.pin_mosi = 23;
	cfg.height = 128;	/* 128x128 device */

	/* Create an instance of the SSD1331 */
	lcd = lcd_ssd1351_init(&cfg);
	if (lcd == NULL)
		exit(127);

	/* Blank out the screen */
	lcd->lcd_clear(lcd, 0);

	/* White bar - left */
	for (x = 0; x < 16; x++)
		lcd->lcd_vline(lcd, x, 0, 63, 0xffffffff);

	/* Bitmap */
	for (y = 0; y < 64; y++) {

		/* Assemble the row data to blit */
		for (x = 0; x < 64; x++) {
			HEADER_PIXEL(hd, col);
			c = ((col[0] << 16) | (col[1] << 8) | (col[2]));
			row_rgb[x] = c;
		}

		/* Blit the whole row at once */
		lcd->lcd_row_blit(lcd, 16, y, row_rgb, 64);
	}

	/* White bar - right */
	for (x = 80; x < 96; x++)
		lcd->lcd_vline(lcd, x, 0, 63, 0xffffffff);

	/* Test putstring */
//	lcd->lcd_putstr(lcd, 0, 0, "Hello, world!", 0x00ff007f, 0);

	exit(0);
}
