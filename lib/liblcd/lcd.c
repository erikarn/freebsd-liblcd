#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <err.h>

#include "lcd.h"

static int
lcd_rowBlit(struct lcd *lcd, int16_t x, int16_t y,
    uint32_t *rgb, int l)
{
	int i;

	/* XXX TODO: bounds checking */

	for (i = 0; i < l; i++) {
		lcd->lcd_pixel(lcd, x + i, y, rgb[i]);
	}

	return (0);
}

static int
lcd_clearScreen(struct lcd *lcd, uint32_t c)
{
	struct lcd_ssd1331 *h = lcd->hw;
	uint16_t y;
	uint16_t color;

	/* Map colour */

	for (y = 0; y < lcd->height; y++) {
		lcd->lcd_line(lcd, 0, y, lcd->width - 1, y, c);
	}

	return (0);
}

struct lcd *
lcd_create(void)
{
	struct lcd *l;

	l = calloc(1, sizeof(*l));
	if (l == NULL) {
		warn("%s: calloc", __func__);
		return (NULL);
	}

	l->lcd_clear = lcd_clearScreen;
	l->lcd_row_blit = lcd_rowBlit;

	return (l);
}

void
lcd_teardown(struct lcd *l)
{

	/* XXX tell HAL layer */
	free(l);
}
