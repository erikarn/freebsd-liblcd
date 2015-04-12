#ifndef	__LCD_H__
#define	__LCD_H__

struct lcd {
	/* TFT width and height */
	int tft_width;
	int tft_height;

	/* Viewport height/width (for rotation, later) */
	int height;
	int width;

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
};

extern	struct lcd * lcd_create(void);
extern	void lcd_teardown(struct lcd *l);

#endif	/* __LCD_H__ */
