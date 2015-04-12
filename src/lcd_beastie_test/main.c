
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h> /* bzero */
#include <libgpio.h>
#include <err.h>

#include "lcd.h"

#include "lcd_ssd1331.h"

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

	/* Create an instance of the SSD1331 */
	lcd = lcd_ssd1331_init();
	if (lcd == NULL)
		exit(127);

	/* Blank out the screen */
	lcd->lcd_clear(lcd, 0);

	/* White bars */
	for (x = 0; x < 16; x++)
		lcd->lcd_line(lcd, x, 0, x, 63, 0xffffffff);

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

	/* White bars */
	for (x = 80; x < 96; x++)
		lcd->lcd_line(lcd, x, 0, x, 63, 0xffffffff);

	exit(0);
}
