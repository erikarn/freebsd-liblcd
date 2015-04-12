/*************************************************** 
  This is a library for the 0.96" 16-bit Color OLED with SSD1331 driver chip

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/products/684

  These displays use SPI to communicate, 4 or 5 pins are required to
  interface
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ****************************************************/

#if 0
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1331.h"
#include "glcdfont.c"

#ifdef __avr__
#include <avr/pgmspace.h>
#endif

#include "pins_arduino.h"
#include "wiring_private.h"
#include <SPI.h>

#endif

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h> /* bzero */
#include <libgpio.h>
#include <err.h>

#include "lcd.h"

#include "lcd_ssd1331_hw.h"
#include "lcd_ssd1331.h"

static inline void
bit_set(struct lcd_ssd1331 *l, gpio_pin_t pin)
{

	gpio_pin_high(l->gp, pin);
	//printf("%s: %d: SET\n", __func__, pin);
}

static inline void
bit_clear(struct lcd_ssd1331 *l, gpio_pin_t pin)
{

	gpio_pin_low(l->gp, pin);
	//printf("%s: %d: CLEAR\n", __func__, pin);
}

#define	_BV(i)		(1 << (i))

/*
 * Low-level pin interface.
 */
static inline void
lcd_ssd1331_spiwrite(struct lcd_ssd1331 *l, uint8_t c)
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
lcd_ssd1331_writeCommand(struct lcd_ssd1331 *l, uint8_t c)
{
	bit_clear(l, l->pin_dc);

	bit_clear(l, l->pin_cs);

	//printf("%02x ", c);
	lcd_ssd1331_spiwrite(l, c);

	bit_set(l,  l->pin_cs);
}


static void
lcd_ssd1331_writeData(struct lcd_ssd1331 *l, uint8_t c)
{

	bit_set(l, l->pin_dc);

	bit_clear(l, l->pin_cs);

    //Serial.print("D ");
	lcd_ssd1331_spiwrite(l, c);

	bit_set(l, l->pin_cs);
}

/***********************************/

static void
lcd_ssd1331_goTo(struct lcd_ssd1331 *l, int x, int y)
{
	if ((x >= l->lcd->width) || (y >= l->lcd->height))
		return;

	/* set x and y coordinate */
	lcd_ssd1331_writeCommand(l, SSD1331_CMD_SETCOLUMN);
	lcd_ssd1331_writeCommand(l, x);
	lcd_ssd1331_writeCommand(l, l->lcd->width - 1);

	lcd_ssd1331_writeCommand(l, SSD1331_CMD_SETROW);
	lcd_ssd1331_writeCommand(l, y);
	lcd_ssd1331_writeCommand(l, l->lcd->height - 1);
}

static uint16_t
lcd_ssd1331_Color565(struct lcd_ssd1331 *l, uint8_t r, uint8_t g, uint8_t b)
{
	uint16_t c;

	c = r >> 3;
	c <<= 6;
	c |= g >> 2;
	c <<= 5;
	c |= b >> 3;

	return (c);
}

static void
lcd_ssd1331_goHome(struct lcd_ssd1331 *l)
{
	lcd_ssd1331_goTo(l, 0, 0);
}


/**************************************************************************/
/*! 
    @brief  Draws a filled rectangle using HW acceleration
*/
/**************************************************************************/
/*
void Adafruit_SSD1331::fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t fillcolor) 
{
//Serial.println("fillRect");
  // check rotation, move rect around if necessary
  switch (getRotation()) {
  case 1:
    swap(x, y);
    swap(w, h);
    x = WIDTH - x - 1;
    break;
  case 2:
    x = WIDTH - x - 1;
    y = HEIGHT - y - 1;
    break;
  case 3:
    swap(x, y);
    swap(w, h);
    y = HEIGHT - y - 1;
    break;
  }

  // Bounds check
  if ((x >= TFTWIDTH) || (y >= TFTHEIGHT))
	return;

  // Y bounds check
  if (y+h > TFTHEIGHT)
  {
    h = TFTHEIGHT - y;
  }

  // X bounds check
  if (x+w > TFTWIDTH)
  {
    w = TFTWIDTH - x;
  }
  
  // fill!
	lcd_ssd1331_writeCommand(SSD1331_CMD_FILL);
	lcd_ssd1331_writeCommand(0x01);

	lcd_ssd1331_writeCommand(SSD1331_CMD_DRAWRECT);
	lcd_ssd1331_writeCommand(x & 0xFF);							// Starting column
	lcd_ssd1331_writeCommand(y & 0xFF);							// Starting row
	lcd_ssd1331_writeCommand((x+w-1) & 0xFF);	// End column
	lcd_ssd1331_writeCommand((y+h-1) & 0xFF);	// End row
  
  // Outline color
	lcd_ssd1331_writeCommand((uint8_t)((fillcolor >> 11) << 1));
	lcd_ssd1331_writeCommand((uint8_t)((fillcolor >> 5) & 0x3F));
	lcd_ssd1331_writeCommand((uint8_t)((fillcolor << 1) & 0x3F));
  // Fill color
	lcd_ssd1331_writeCommand((uint8_t)((fillcolor >> 11) << 1));
	lcd_ssd1331_writeCommand((uint8_t)((fillcolor >> 5) & 0x3F));
	lcd_ssd1331_writeCommand((uint8_t)((fillcolor << 1) & 0x3F));
 
  // Delay while the fill completes
  delay(SSD1331_DELAYS_HWFILL); 
}
*/

static int
lcd_ssd1331_drawLine(struct lcd *lcd, int16_t x0, int16_t y0,
    int16_t x1, int16_t y1, uint32_t c)
{
	struct lcd_ssd1331 *l = lcd->hw;
	uint16_t color;

	color = lcd_ssd1331_Color565(l,
	    (c >> 16) & 0xff,
	    (c >> 8) & 0xff,
	    (c) & 0xff);


#if 0
  // check rotation, move pixel around if necessary
  switch (getRotation()) {
  case 1:
    swap(x0, y0);
    swap(x1, y1);
    x0 = WIDTH - x0 - 1;
    x1 = WIDTH - x1 - 1;
    break;
  case 2:
    x0 = WIDTH - x0 - 1;
    y0 = HEIGHT - y0 - 1;
    x1 = WIDTH - x1 - 1;
    y1 = HEIGHT - y1 - 1;
    break;
  case 3:
    swap(x0, y0);
    swap(x1, y1);
    y0 = HEIGHT - y0 - 1;
    y1 = HEIGHT - y1 - 1;
    break;
  }
#endif

	// Boundary check
	if ((y0 >= l->lcd->height) && (y1 >= l->lcd->height))
		return (-1);
	if ((x0 >= l->lcd->width) && (x1 >= l->lcd->width))
		return (-1);
#if 0
  if (x0 >= TFTWIDTH)
    x0 = TFTWIDTH - 1;
  if (y0 >= TFTHEIGHT)
    y0 = TFTHEIGHT - 1;
  if (x1 >= TFTWIDTH)
    x1 = TFTWIDTH - 1;
  if (y1 >= TFTHEIGHT)
    y1 = TFTHEIGHT - 1;
#endif

	lcd_ssd1331_writeCommand(l, SSD1331_CMD_DRAWLINE);
	lcd_ssd1331_writeCommand(l, x0);
	lcd_ssd1331_writeCommand(l, y0);
	lcd_ssd1331_writeCommand(l, x1);
	lcd_ssd1331_writeCommand(l, y1);
	usleep(SSD1331_DELAYS_HWLINE * 1000);
	lcd_ssd1331_writeCommand(l, (uint8_t)((color >> 11) << 1));
	lcd_ssd1331_writeCommand(l, (uint8_t)((color >> 5) & 0x3F));
	lcd_ssd1331_writeCommand(l, (uint8_t)((color << 1) & 0x3F));
	usleep(SSD1331_DELAYS_HWLINE * 1000);

	return (0);
}

static int
lcd_ssd1331_drawPixel(struct lcd *lcd, int16_t x, int16_t y,
    uint32_t c)
{
	struct lcd_ssd1331 *l = lcd->hw;
	uint16_t color;

#if 0
if ((x < 0) || (x >= width()) || (y < 0) || (y >= height())) return;

  // check rotation, move pixel around if necessary
  switch (getRotation()) {
  case 1:
    swap(x, y);
    x = WIDTH - x - 1;
    break;
  case 2:
    x = WIDTH - x - 1;
    y = HEIGHT - y - 1;
    break;
  case 3:
    swap(x, y);
    y = HEIGHT - y - 1;
    break;
  }
#endif

	/* XXX Boundary check */

	color = lcd_ssd1331_Color565(l,
	    (c >> 16) & 0xff,
	    (c >> 8) & 0xff,
	    (c) & 0xff);

	lcd_ssd1331_goTo(l, x, y);

	// setup for data
	/* XXX use the datawrite method? */
	bit_set(l, l->pin_dc);
	bit_clear(l, l->pin_cs);

	lcd_ssd1331_spiwrite(l, color >> 8);
	lcd_ssd1331_spiwrite(l, color);

	bit_set(l, l->pin_cs);

	return (0);
}

void
lcd_ssd1331_pushColor(struct lcd_ssd1331 *l, uint16_t color)
{
	// setup for data
	/* XXX use datawrite method? */
	bit_set(l, l->pin_dc);
	bit_clear(l, l->pin_cs);

	lcd_ssd1331_spiwrite(l, color >> 8);
	lcd_ssd1331_spiwrite(l, color);

	bit_set(l, l->pin_cs);
}


void
lcd_ssd1331_begin(struct lcd_ssd1331 *l)
{
	/* XXX TODO: GPIO setup */

	// Toggle RST low to reset; CS low so it'll listen to us
	bit_clear(l, l->pin_cs);

	bit_set(l, l->pin_rst);
	usleep(500 * 1000);
	bit_clear(l, l->pin_rst);
	usleep(500 * 1000);
	bit_set(l, l->pin_rst);
	usleep(500 * 1000);

	// Initialization Sequence
	lcd_ssd1331_writeCommand(l, SSD1331_CMD_DISPLAYOFF);  	// 0xAE
	lcd_ssd1331_writeCommand(l, SSD1331_CMD_SETREMAP); 	// 0xA0
#if defined SSD1331_COLORORDER_RGB
	lcd_ssd1331_writeCommand(l, 0x72);				// RGB Color
#else
	lcd_ssd1331_writeCommand(l, 0x76);				// BGR Color
#endif
	lcd_ssd1331_writeCommand(l, SSD1331_CMD_STARTLINE); 	// 0xA1
	lcd_ssd1331_writeCommand(l, 0x0);
	lcd_ssd1331_writeCommand(l, SSD1331_CMD_DISPLAYOFFSET); 	// 0xA2
	lcd_ssd1331_writeCommand(l, 0x0);
	lcd_ssd1331_writeCommand(l, SSD1331_CMD_NORMALDISPLAY);  	// 0xA4
	lcd_ssd1331_writeCommand(l, SSD1331_CMD_SETMULTIPLEX); 	// 0xA8
	lcd_ssd1331_writeCommand(l, 0x3F);  			// 0x3F 1/64 duty
	lcd_ssd1331_writeCommand(l, SSD1331_CMD_SETMASTER);  	// 0xAD
	lcd_ssd1331_writeCommand(l, 0x8E);
	lcd_ssd1331_writeCommand(l, SSD1331_CMD_POWERMODE);  	// 0xB0
	lcd_ssd1331_writeCommand(l, 0x0B);
	lcd_ssd1331_writeCommand(l, SSD1331_CMD_PRECHARGE);  	// 0xB1
	lcd_ssd1331_writeCommand(l, 0x31);
	lcd_ssd1331_writeCommand(l, SSD1331_CMD_CLOCKDIV);  	// 0xB3
	lcd_ssd1331_writeCommand(l, 0xF0);  // 7:4 = Oscillator Frequency, 3:0 = CLK Div Ratio (A[3:0]+1 = 1..16)
	lcd_ssd1331_writeCommand(l, SSD1331_CMD_PRECHARGEA);  	// 0x8A
	lcd_ssd1331_writeCommand(l, 0x64);
	lcd_ssd1331_writeCommand(l, SSD1331_CMD_PRECHARGEB);  	// 0x8B
	lcd_ssd1331_writeCommand(l, 0x78);
	lcd_ssd1331_writeCommand(l, SSD1331_CMD_PRECHARGEA);  	// 0x8C
	lcd_ssd1331_writeCommand(l, 0x64);
	lcd_ssd1331_writeCommand(l, SSD1331_CMD_PRECHARGELEVEL);  	// 0xBB
	lcd_ssd1331_writeCommand(l, 0x3A);
	lcd_ssd1331_writeCommand(l, SSD1331_CMD_VCOMH);  		// 0xBE
	lcd_ssd1331_writeCommand(l, 0x3E);
	lcd_ssd1331_writeCommand(l, SSD1331_CMD_MASTERCURRENT);  	// 0x87
	lcd_ssd1331_writeCommand(l, 0x06);
	lcd_ssd1331_writeCommand(l, SSD1331_CMD_CONTRASTA);  	// 0x81
	lcd_ssd1331_writeCommand(l, 0x91);
	lcd_ssd1331_writeCommand(l, SSD1331_CMD_CONTRASTB);  	// 0x82
	lcd_ssd1331_writeCommand(l, 0x50);
	lcd_ssd1331_writeCommand(l, SSD1331_CMD_CONTRASTC);  	// 0x83
	lcd_ssd1331_writeCommand(l, 0x7D);
	lcd_ssd1331_writeCommand(l, SSD1331_CMD_DISPLAYON);	//--turn on oled panel    
}

#if 0
/********************************* low level pin initialization */

Adafruit_SSD1331::Adafruit_SSD1331(uint8_t cs, uint8_t rs, uint8_t sid, uint8_t sclk, uint8_t rst) : Adafruit_GFX(TFTWIDTH, TFTHEIGHT) {
    _cs = cs;
    _rs = rs;
    _sid = sid;
    _sclk = sclk;
    _rst = rst;
}

Adafruit_SSD1331::Adafruit_SSD1331(uint8_t cs, uint8_t rs, uint8_t rst) : Adafruit_GFX(TFTWIDTH, TFTHEIGHT) {
    _cs = cs;
    _rs = rs;
    _sid = 0;
    _sclk = 0;
    _rst = rst;
}
#endif

int
lcd_ssd1331_init(struct lcd *l)
{
	struct lcd_ssd1331 *h;

	h = calloc(1, sizeof(*h));
	if (h == NULL) {
		warn("%s: calloc", __func__);
		return (-1);
	}

	/* forward/back pointers */
	l->hw = h;
	h->lcd = l;

	h->gp = gpio_open(0);
	if (h->gp == GPIO_VALUE_INVALID) {
		warn("%s: gpio_open", __func__);
		return (-1);
	}

	/* HAL initialisation */
	l->tft_width = 96;
	l->tft_height = 64;
	/* NB: no rotation support yet */
	l->width = 96;
	l->height = 64;
	l->lcd_pixel = lcd_ssd1331_drawPixel;
	l->lcd_line = lcd_ssd1331_drawLine;

	/* XXX hard-coded gpio pins for testing */
	h->pin_cs = 19;
	h->pin_rst = 20;
	h->pin_dc = 21;
	h->pin_sck = 22;
	h->pin_mosi = 23;

	/* Configure as outputs */
	(void) gpio_pin_output(h->gp, h->pin_cs);
	(void) gpio_pin_output(h->gp, h->pin_rst);
	(void) gpio_pin_output(h->gp, h->pin_dc);
	(void) gpio_pin_output(h->gp, h->pin_sck);
	(void) gpio_pin_output(h->gp, h->pin_mosi);

	/* Initialise LCD */
	lcd_ssd1331_begin(h);

	return (0);
}

void
lcd_ssd1331_xline(struct lcd *lcd, int16_t x, uint32_t c)
{
	int16_t y;

	/* XXX bounds check */
	for (y = 0; y < lcd->height; y++) {
			lcd_ssd1331_drawPixel(lcd, x, y, c);
	}
}
