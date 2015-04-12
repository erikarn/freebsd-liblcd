#ifndef	__LCD_SSD1331_H__
#define	__LCD_SSD1331_H__

struct lcd_ssd1331 {
	struct lcd *lcd;

	gpio_handle_t gp;
	gpio_pin_t pin_cs;	/* chip select */
	gpio_pin_t pin_rst;	/* chip reset */
	gpio_pin_t pin_dc;	/* data/command */
	gpio_pin_t pin_sck;	/* spi clock */
	gpio_pin_t pin_mosi;	/* spi data */
	/* XXX no miso wired up */
};

extern	struct lcd * lcd_ssd1331_init(void);

#endif	/* __LCD_SSD1331_H__ */
