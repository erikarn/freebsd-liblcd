#ifndef	__LCD_H__
#define	__LCD_H__

struct lcd {
	/* TFT width and height */
	int tft_width;
	int tft_height;

	/* HAL local state pointer */
	void *hw;

	/* API - set by the LCD HAL */

	/* Plot a pixel - c is 'xxRRGGBB' format */
	int (*lcd_pixel)(struct lcd *l, int16_t x, int16_t y, uint32_t c);

	/*
	 * XXX TODO: other methods; some of which will
	 * have LCD accelerated versions and may be overridden
	 * by the LCD HAL.
	 */

	/* Draw a line */
	int (*lcd_line) (struct lcd *lcd, int16_t x0, int16_t y0,
	    int16_t x1, int16_t y1, uint32_t c);

	/* Clear the screen to colour 'c' */
	int (*lcd_clear) (struct lcd *lcd, uint32_t c);

	/* Plot the row of RGB data at x, y, length l */
	int (*lcd_row_blit) (struct lcd *lcd, int16_t x, int16_t y,
	    uint32_t *rgb, int l);

	/* Write character at given x, y */
	int (* lcd_putchar) (struct lcd *lcd, int16_t x0, int16_t y0,
	    unsigned char c, uint32_t fg, uint32_t bg);

	/* Write string at given x, y */
	int (* lcd_putstr) (struct lcd *lcd, int16_t x0, int16_t y0,
	    const unsigned char *s, uint32_t fg, uint32_t bg);
};

extern	struct lcd * lcd_create(void);
extern	void lcd_teardown(struct lcd *l);

#endif	/* __LCD_H__ */
