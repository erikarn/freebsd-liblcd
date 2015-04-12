/*************************************************** 
  This is a library for the 1.5" & 1.27" 16-bit Color OLEDs 
  with SSD1331 driver chip

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/products/1431
  ------> http://www.adafruit.com/products/1673

  These displays use SPI to communicate, 4 or 5 pins are required to  
  interface
  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
 ****************************************************/


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h> /* bzero */
#include <libgpio.h>
#include <err.h>

#include "lcd.h"

#include "lcd_ssd1351_hw.h"
#include "lcd_ssd1351.h"

/********************************** low level pin interface */

/* XXX duplicated from lcd_ssd1331.c; should refactor */
static inline void
bit_set(struct lcd_ssd1351 *l, gpio_pin_t pin)
{

	gpio_pin_high(l->gp, pin);
	//printf("%s: %d: SET\n", __func__, pin);
}

static inline void
bit_clear(struct lcd_ssd1351 *l, gpio_pin_t pin)
{

	gpio_pin_low(l->gp, pin);
	//printf("%s: %d: CLEAR\n", __func__, pin);
}

#define	_BV(i)		(1 << (i))

/*
 * Low-level pin interface.
 */
static inline void
lcd_ssd1351_spiwrite(struct lcd_ssd1351 *l, uint8_t c)
{
	int i;

	bit_set(l, l->pin_sck);

	for (i = 7; i >= 0; i--) {
		bit_clear(l, l->pin_sck);

		if (c & _BV(i)) {
			bit_set(l, l->pin_mosi);
		} else {
			bit_clear(l, l->pin_mosi);
		}

		bit_set(l, l->pin_sck);
	}
}

#undef	_BV


static void
lcd_ssd1351_writeCommand(struct lcd_ssd1351 *l, uint8_t c)
{
	bit_clear(l, l->pin_dc);
	bit_clear(l, l->pin_cs);

	//printf("%02x ", c);
	lcd_ssd1351_spiwrite(l, c);

	bit_set(l,  l->pin_cs);
}

static void
lcd_ssd1351_writeData(struct lcd_ssd1351 *l, uint8_t c)
{

	bit_set(l, l->pin_dc);
	bit_clear(l, l->pin_cs);

	//Serial.print("D ");
	lcd_ssd1351_spiwrite(l, c);

	bit_set(l, l->pin_cs);
}

/***********************************/

static void
lcd_ssd1351_goTo(struct lcd_ssd1351 *l, int x, int y)
{

	if ((x >= l->lcd->tft_width ) || (y >= l->lcd->tft_height))
		return;

	// set x and y coordinate
	lcd_ssd1351_writeCommand(l, SSD1351_CMD_SETCOLUMN);
	lcd_ssd1351_writeData(l, x);
	lcd_ssd1351_writeData(l, SSD1351WIDTH-1);

	lcd_ssd1351_writeCommand(l, SSD1351_CMD_SETROW);
	lcd_ssd1351_writeData(l, y);
	lcd_ssd1351_writeData(l, SSD1351HEIGHT-1);

	lcd_ssd1351_writeCommand(l, SSD1351_CMD_WRITERAM);
}

static uint16_t
lcd_ssd1351_Color565(uint8_t r, uint8_t g, uint8_t b)
{
	uint16_t c;
	c = r >> 3;
	c <<= 6;
	c |= g >> 2;
	c <<= 5;
	c |= b >> 3;

	return (c);
}

static int
lcd_ssd1351_drawPixel(struct lcd *lcd, int16_t x, int16_t y, uint32_t c)
{
	struct lcd_ssd1351 *l;
	uint16_t color;

	l = lcd->hw;

	color = lcd_ssd1351_Color565((c >> 16) & 0xff,
	    (c >> 8) & 0xff,
	    (c) & 0xff);

	// Bounds check.
	if ((x >= lcd->tft_width) || (y >= lcd->tft_height))
		return (-1);
	if ((x < 0) || (y < 0))
		return (-1);

	/* Set initial location/bounds */
	lcd_ssd1351_goTo(l, x, y);

	// setup for data
	bit_set(l, l->pin_dc);
	bit_clear(l, l->pin_cs);

	lcd_ssd1351_spiwrite(l, color >> 8);
	lcd_ssd1351_spiwrite(l, color);

	bit_set(l, l->pin_cs);

	return (0);
}

/*
 * Draw a horizontal line.
 */
static int
lcd_ssd1351_rawFastHLine(struct lcd *lcd, int16_t x0, int16_t x1, int16_t y0,
    uint32_t c)
{
	struct lcd_ssd1351 *l;
	uint16_t color;
	int i;
	int16_t w;

	l = lcd->hw;

	/* Map colour */
	color = lcd_ssd1351_Color565((c >> 16) & 0xff,
	    (c >> 8) & 0xff,
	    (c) & 0xff);

	/* XXX bounds checking */

	/* Calculate width */
	w = x1 - x0 + 1;

	// set location
	lcd_ssd1351_writeCommand(l, SSD1351_CMD_SETCOLUMN);
	lcd_ssd1351_writeData(l, x0);
	lcd_ssd1351_writeData(l, x1);
	lcd_ssd1351_writeCommand(l, SSD1351_CMD_SETROW);
	lcd_ssd1351_writeData(l, y0);
	lcd_ssd1351_writeData(l, y0);
	// fill!
	lcd_ssd1351_writeCommand(l, SSD1351_CMD_WRITERAM);

	for (i = 0; i < w; i++) {
		lcd_ssd1351_writeData(l, color >> 8);
		lcd_ssd1351_writeData(l, color);
	}

	return (0);
}
/*
 * Draw a vertical line.
 */
static int
lcd_ssd1351_rawFastVLine(struct lcd *lcd, int16_t x0, int16_t y0, int16_t y1,
    uint32_t c)
{
	struct lcd_ssd1351 *l;
	uint16_t color;
	int i;
	int16_t h;

	l = lcd->hw;

	/* Map colour */
	color = lcd_ssd1351_Color565((c >> 16) & 0xff,
	    (c >> 8) & 0xff,
	    (c) & 0xff);

	/* XXX bounds checking */

	/* Height */
	h = y1 - y0 + 1;

	// set location
	lcd_ssd1351_writeCommand(l, SSD1351_CMD_SETCOLUMN);
	lcd_ssd1351_writeData(l, x0);
	lcd_ssd1351_writeData(l, x0);
	lcd_ssd1351_writeCommand(l, SSD1351_CMD_SETROW);
	lcd_ssd1351_writeData(l, y0);
	lcd_ssd1351_writeData(l, y1);
	// fill!
	lcd_ssd1351_writeCommand(l, SSD1351_CMD_WRITERAM);

	for (i = 0; i < h; i++) {
		lcd_ssd1351_writeData(l, color >> 8);
		lcd_ssd1351_writeData(l, color);
	}

	return (0);
}

static int
lcd_ssd1351_rowBlit(struct lcd *lcd, int16_t x, int16_t y,
    uint32_t *rgb, int l)
{
	struct lcd_ssd1351 *h = lcd->hw;
	int i;
	uint16_t color;
	uint32_t c;

	/* Set starting point */
	lcd_ssd1351_goTo(h, x, y);

	/* Push lots of colours */
	lcd_ssd1351_writeCommand(h, SSD1351_CMD_WRITERAM);
	for (i = 0; i < l; i++) {
		c = rgb[i];
		color = lcd_ssd1351_Color565((c >> 16) & 0xff,
		    (c >> 8) & 0xff,
		    (c) & 0xff);
		lcd_ssd1351_writeData(h, color >> 8);
		lcd_ssd1351_writeData(h, color);
	}

	return (0);
}

static int
lcd_ssd1351_begin(struct lcd_ssd1351 *l)
{

	// Toggle RST low to reset; CS low so it'll listen to us
	bit_clear(l, l->pin_cs);

	bit_set(l, l->pin_rst);
	usleep(500 * 1000);
	bit_clear(l, l->pin_rst);
	usleep(500 * 1000);
	bit_set(l, l->pin_rst);
	usleep(500 * 1000);

	// Initialization Sequence
	lcd_ssd1351_writeCommand(l, SSD1351_CMD_COMMANDLOCK);  // set command lock
	lcd_ssd1351_writeData(l, 0x12);
	lcd_ssd1351_writeCommand(l, SSD1351_CMD_COMMANDLOCK);  // set command lock
	lcd_ssd1351_writeData(l, 0xB1);

	lcd_ssd1351_writeCommand(l, SSD1351_CMD_DISPLAYOFF);  		// 0xAE

	lcd_ssd1351_writeCommand(l, SSD1351_CMD_CLOCKDIV);  		// 0xB3
	lcd_ssd1351_writeCommand(l, 0xF1);  						// 7:4 = Oscillator Frequency, 3:0 = CLK Div Ratio (A[3:0]+1 = 1..16)

	lcd_ssd1351_writeCommand(l, SSD1351_CMD_MUXRATIO);
	lcd_ssd1351_writeData(l, 127);

	lcd_ssd1351_writeCommand(l, SSD1351_CMD_SETREMAP);
	lcd_ssd1351_writeData(l, 0x74);

	lcd_ssd1351_writeCommand(l, SSD1351_CMD_SETCOLUMN);
	lcd_ssd1351_writeData(l, 0x00);
	lcd_ssd1351_writeData(l, 0x7F);
	lcd_ssd1351_writeCommand(l, SSD1351_CMD_SETROW);
	lcd_ssd1351_writeData(l, 0x00);
	lcd_ssd1351_writeData(l, 0x7F);

	lcd_ssd1351_writeCommand(l, SSD1351_CMD_STARTLINE); 		// 0xA1
	if (l->lcd->tft_height == 96) {
		lcd_ssd1351_writeData(l, 96);
	} else {
		lcd_ssd1351_writeData(l, 0);
	}

	lcd_ssd1351_writeCommand(l, SSD1351_CMD_DISPLAYOFFSET); 	// 0xA2
	lcd_ssd1351_writeData(l, 0x0);

	lcd_ssd1351_writeCommand(l, SSD1351_CMD_SETGPIO);
	lcd_ssd1351_writeData(l, 0x00);

	lcd_ssd1351_writeCommand(l, SSD1351_CMD_FUNCTIONSELECT);
	lcd_ssd1351_writeData(l, 0x01); // internal (diode drop)
	//writeData(0x01); // external bias

	//writeCommand(SSSD1351_CMD_SETPHASELENGTH);
	//writeData(0x32);

	lcd_ssd1351_writeCommand(l, SSD1351_CMD_PRECHARGE);  		// 0xB1
	lcd_ssd1351_writeCommand(l, 0x32);

	lcd_ssd1351_writeCommand(l, SSD1351_CMD_VCOMH);  			// 0xBE
	lcd_ssd1351_writeCommand(l, 0x05);

	lcd_ssd1351_writeCommand(l, SSD1351_CMD_NORMALDISPLAY);  	// 0xA6

	lcd_ssd1351_writeCommand(l, SSD1351_CMD_CONTRASTABC);
	lcd_ssd1351_writeData(l, 0xC8);
	lcd_ssd1351_writeData(l, 0x80);
	lcd_ssd1351_writeData(l, 0xC8);

	lcd_ssd1351_writeCommand(l, SSD1351_CMD_CONTRASTMASTER);
	lcd_ssd1351_writeData(l, 0x0F);

	lcd_ssd1351_writeCommand(l, SSD1351_CMD_SETVSL );
	lcd_ssd1351_writeData(l, 0xA0);
	lcd_ssd1351_writeData(l, 0xB5);
	lcd_ssd1351_writeData(l, 0x55);

	lcd_ssd1351_writeCommand(l, SSD1351_CMD_PRECHARGE2);
	lcd_ssd1351_writeData(l, 0x01);

	lcd_ssd1351_writeCommand(l, SSD1351_CMD_DISPLAYON);		//--turn on oled panel

	return (0);
}

struct lcd *
lcd_ssd1351_init(struct lcd_ssd1351_cfg *cfg)
{
	struct lcd_ssd1351 *h = 0;
	struct lcd *l = 0;

	/* Create an LCD handle */
	l = lcd_create();
	if (l == NULL) {
		warn("%s: calloc (lcd)", __func__);
		goto error;
	}

	h = calloc(1, sizeof(*h));
	if (h == NULL) {
		warn("%s: calloc", __func__);
		goto error;
	}

	/* forward/back pointers */
	l->hw = h;
	h->lcd = l;

	/* HAL: create GPIO handle */
	h->gp = gpio_open(cfg->gpio_unit);
	if (h->gp == GPIO_VALUE_INVALID) {
		warn("%s: gpio_open", __func__);
		goto error;
	}

	/* Initialise LCD layer with parameters and hardware methods */
	l->tft_width = 128;
	l->tft_height = cfg->height;
	l->lcd_pixel = lcd_ssd1351_drawPixel;
	l->lcd_hline = lcd_ssd1351_rawFastHLine;
	l->lcd_vline = lcd_ssd1351_rawFastVLine;
	l->lcd_row_blit = lcd_ssd1351_rowBlit;

	h->pin_cs = cfg->pin_cs;
	h->pin_rst = cfg->pin_rst;
	h->pin_dc = cfg->pin_dc;
	h->pin_sck = cfg->pin_sck;
	h->pin_mosi = cfg->pin_mosi;

	/* Configure as outputs */
	(void) gpio_pin_output(h->gp, h->pin_cs);
	(void) gpio_pin_output(h->gp, h->pin_rst);
	(void) gpio_pin_output(h->gp, h->pin_dc);
	(void) gpio_pin_output(h->gp, h->pin_sck);
	(void) gpio_pin_output(h->gp, h->pin_mosi);

	/* Initialise LCD */
	lcd_ssd1351_begin(h);

	return (l);
error:
	/* XXX TODO: methods */
	if (h) {
		if (h->gp != GPIO_VALUE_INVALID)
			gpio_close(h->gp);
		free(h);
	}
	free(l);
	return (NULL);
}
