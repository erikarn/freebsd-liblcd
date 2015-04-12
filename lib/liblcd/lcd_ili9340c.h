#ifndef	__LCD_ILI9340C_H__
#define	__LCD_ILI9340C_H__

/*
 * This is for the 240x320 2.2" TFT module from Adafruit.
 */
struct lcd_ili9340c_cfg {
	/*
	 * For now, assume that all of the pins
	 * for this are on the same GPIO bus.
	 *
	 * It's not hard to abstract that out if
	 * required.
	 */
	int gpio_unit;
	int pin_cs;
	int pin_rst;
	int pin_dc;
	int pin_sck;
	int pin_mosi;
	/* XXX no MISO pin wired up */
};

struct lcd_ili9340c {
	struct lcd *lcd;

	gpio_handle_t gp;
	gpio_pin_t pin_cs;	/* chip select */
	gpio_pin_t pin_rst;	/* chip reset */
	gpio_pin_t pin_dc;	/* data/command */
	gpio_pin_t pin_sck;	/* spi clock */
	gpio_pin_t pin_mosi;	/* spi data */
	/* XXX no miso wired up */
};

extern	struct lcd * lcd_ili9340c_init(struct lcd_ili9340c_cfg *cfg);

#endif	/* __LCD_ILI9340C_H__ */
