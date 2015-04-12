#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <err.h>

#include "font_5x7.h"

#include "lcd.h"

#define	_BV(i)		(1 << (i))
/*
 * Simple font renderer at the given pixel position.
 */
static int
lcd_putChar(struct lcd *lcd, int16_t x0, int16_t y0,
    unsigned char c, uint32_t fg, uint32_t bg)
{
	int x, y;
	int o;
	char ch;

	/* XXX for now, just assume 5x7 font */

	/* o: start byte in the font array for the character 'c' */
	/*
	 * The font is arranged as 5 bytes per character, but it's 5x7;
	 * so each byte is actually the 7 pixels along the 'y' axis.
	 * This is .. completely backwards to the normal way this
	 * stuff is arranged - it's more space saving (which makes sense
	 * as it's crimed from the arduino adafruit LCD library)
	 * but it's more annoying to render efficiently.
	 */
	o = c * 5;

	/* XXX TODO: bounds checking */
	for (x = 0; x < 6; x++) {
		for (y = 0; y < 8; y++) {
			/* Last column - always background colour */
			if (x == 5)
				ch = 0;
			else
				ch = font_5x7[o + x];
			if (ch & _BV(y)) {
				lcd->lcd_pixel(lcd, x0 + x, y0 + y,
				    fg);
			} else {
				lcd->lcd_pixel(lcd, x0 + x, y0 + y,
				    bg);
			}
		}
	}
	return (0);
}
#undef	_BV

static int
lcd_putStr(struct lcd *lcd, int16_t x0, int16_t y0,
    const unsigned char *s, uint32_t fg, uint32_t bg)
{
	uint16_t x = 0;

	/*
	 * XXX TODO: more bounds checking!
	 */
	while (*s != '\0') {
		if (x + x0 >= lcd->tft_width)
			break;
		lcd_putChar(lcd, x0 + x, y0, *s, fg, bg);
		s++;
		x += 6;
	}

	return (0);
}

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

	for (y = 0; y < lcd->tft_height; y++) {
		lcd->lcd_line(lcd, 0, y, lcd->tft_width - 1, y, c);
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
	l->lcd_putchar = lcd_putChar;
	l->lcd_putstr = lcd_putStr;

	return (l);
}

void
lcd_teardown(struct lcd *l)
{

	/* XXX tell HAL layer */
	free(l);
}
