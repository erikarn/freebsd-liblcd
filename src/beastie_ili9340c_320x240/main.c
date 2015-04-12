
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h> /* bzero */
#include <libgpio.h>
#include <err.h>

#include "lcd.h"

#include "lcd_ili9340c.h"

#include "freebsd-128x128.h"

int
main(int argc, const char *argv[])
{
	int16_t x, y;
	uint32_t c;
	const char *hd = header_data;
	uint8_t col[3];
	struct lcd *lcd;
	uint32_t row_rgb[128];
	struct lcd_ili9340c_cfg cfg;

	/* Configured for the carambola 2 board */
	cfg.gpio_unit = 0;
	cfg.pin_cs = 19;
	cfg.pin_rst = 20;
	cfg.pin_dc = 21;
	cfg.pin_sck = 22;
	cfg.pin_mosi = 23;

	/* Create an instance of the SSD1331 */
	lcd = lcd_ili9340c_init(&cfg);
	if (lcd == NULL)
		exit(127);

	/* Blank out the screen */
	lcd->lcd_clear(lcd, 0);

	/* Bitmap */
	for (y = 0; y < 128; y++) {

		/* Assemble the row data to blit */
		for (x = 0; x < 128; x++) {
			HEADER_PIXEL(hd, col);
			c = ((col[0] << 16) | (col[1] << 8) | (col[2]));
			row_rgb[x] = c;
		}

		/* Blit the whole row at once */
		lcd->lcd_row_blit(lcd, 0, y, row_rgb, 128);
	}

	/* Test putstring */
//	lcd->lcd_putstr(lcd, 0, 0, "Hello, world!", 0x00ff007f, 0);

	exit(0);
}
