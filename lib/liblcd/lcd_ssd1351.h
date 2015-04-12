#ifndef	__LCD_SSD1351_H__
#define	__LCD_SSD1351_H__

/*
 * The Adafruit 128x128 and 128x96 display exposes the
 * SPI interface rather than the 8080/6800 MCU
 * interface.
 *
 * If at some point someone desires the MCU interfaces,
 * it shouldn't be hard to add - the bus code would need
 * abstrading in lcd_ssd1331.c but once that's done, the
 * rest of the LCD driver will just work.
 */
struct lcd_ssd1351_cfg {
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

	/* There's a 96 and 128 pixel high version. */
	int height;
};

struct lcd_ssd1351 {
	struct lcd *lcd;

	gpio_handle_t gp;
	gpio_pin_t pin_cs;	/* chip select */
	gpio_pin_t pin_rst;	/* chip reset */
	gpio_pin_t pin_dc;	/* data/command */
	gpio_pin_t pin_sck;	/* spi clock */
	gpio_pin_t pin_mosi;	/* spi data */
	/* XXX no miso wired up */
};

extern	struct lcd * lcd_ssd1351_init(struct lcd_ssd1351_cfg *cfg);

#endif	/* __LCD_SSD1351_H__ */
