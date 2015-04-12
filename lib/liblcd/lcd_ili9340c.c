/***************************************************
  This is an Arduino Library for the Adafruit 2.2" SPI display.
  This library works with the Adafruit 2.2" TFT Breakout w/SD card
  ----> http://www.adafruit.com/products/1480

  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h> /* bzero */
#include <libgpio.h>
#include <err.h>

#include "lcd.h"

#include "lcd_ili9340c_hw.h"
#include "lcd_ili9340c.h"

/********************************** low level pin interface */

/* XXX duplicated from lcd_ssd1331.c; should refactor */
static inline void
bit_set(struct lcd_ili9340c *l, gpio_pin_t pin)
{

	gpio_pin_high(l->gp, pin);
	//printf("%s: %d: SET\n", __func__, pin);
}

static inline void
bit_clear(struct lcd_ili9340c *l, gpio_pin_t pin)
{

	gpio_pin_low(l->gp, pin);
	//printf("%s: %d: CLEAR\n", __func__, pin);
}

#define	_BV(i)		(1 << (i))

/*
 * Low-level pin interface - different from others.
 * XXX TODO: see if this is required?
 */
static inline void
lcd_ili9340c_spiwrite(struct lcd_ili9340c *l, uint8_t c)
{
	int i;

	for (i = 7; i >= 0; i--) {
		if (c & _BV(i)) {
			bit_set(l, l->pin_mosi);
		} else {
			bit_clear(l, l->pin_mosi);
		}
		bit_set(l, l->pin_sck);
		/* XXX delay? */
		bit_clear(l, l->pin_sck);
	}
}

#undef	_BV

#if 0
void Adafruit_ILI9340::spiwrite(uint8_t c) {

  //Serial.print("0x"); Serial.print(c, HEX); Serial.print(", ");

    // Fast SPI bitbang swiped from LPD8806 library
    for(uint8_t bit = 0x80; bit; bit >>= 1) {
      if(c & bit) {
        //digitalWrite(_mosi, HIGH);
        SET_BIT(mosiport, mosipinmask);
      } else {
        //digitalWrite(_mosi, LOW);
        CLEAR_BIT(mosiport, mosipinmask);
      }
      //digitalWrite(_sclk, HIGH);
      SET_BIT(clkport, clkpinmask);
      //digitalWrite(_sclk, LOW);
      CLEAR_BIT(clkport, clkpinmask);
    }
}
#endif


static void
lcd_ili9340c_writecommand(struct lcd_ili9340c *l, uint8_t c)
{
	bit_clear(l, l->pin_dc);
	bit_clear(l, l->pin_sck);
	bit_clear(l, l->pin_cs);

	//printf("%02x ", c);
	lcd_ili9340c_spiwrite(l, c);

	bit_set(l,  l->pin_cs);
}

static void
lcd_ili9340c_writedata(struct lcd_ili9340c *l, uint8_t c)
{

	bit_set(l, l->pin_dc);
	bit_clear(l, l->pin_sck);
	bit_clear(l, l->pin_cs);

	//Serial.print("D ");
	lcd_ili9340c_spiwrite(l, c);

	bit_set(l, l->pin_cs);
}

/***********************************/

static int
lcd_ili9340c_setAddrWindow(struct lcd_ili9340c *l,
    uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{

	lcd_ili9340c_writecommand(l, ILI9340_CASET); // Column addr set
	lcd_ili9340c_writedata(l, x0 >> 8);
	lcd_ili9340c_writedata(l, x0 & 0xFF);     // XSTART
	lcd_ili9340c_writedata(l, x1 >> 8);
	lcd_ili9340c_writedata(l, x1 & 0xFF);     // XEND

	lcd_ili9340c_writecommand(l, ILI9340_PASET); // Row addr set
	lcd_ili9340c_writedata(l, y0>>8);
	lcd_ili9340c_writedata(l, y0);     // YSTART
	lcd_ili9340c_writedata(l, y1>>8);
	lcd_ili9340c_writedata(l, y1);     // YEND

	lcd_ili9340c_writecommand(l, ILI9340_RAMWR); // write to RAM
	return (0);
}

// Pass 8-bit (each) R,G,B, get back 16-bit packed color
static inline uint16_t
lcd_ili9340c_Color565(uint8_t r, uint8_t g, uint8_t b)
{
	return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

static int
lcd_ili9340c_pushColor(struct lcd_ili9340c *l, uint16_t color)
{
	bit_set(l, l->pin_dc);
	bit_clear(l, l->pin_cs);

	lcd_ili9340c_spiwrite(l, color >> 8);
	lcd_ili9340c_spiwrite(l, color);

	bit_set(l, l->pin_cs);

	return (0);
}

static int
lcd_ili9340c_drawPixel(struct lcd *lcd, int16_t x, int16_t y,
    uint32_t c)
{
	struct lcd_ili9340c *l;
	uint16_t color;

	/* XXX boundary check */
	l = lcd->hw;

	color = lcd_ili9340c_Color565((c >> 16) & 0xff,
	    (c >> 8) & 0xff,
	    (c) & 0xff);

	lcd_ili9340c_setAddrWindow(l, x,y,x+1,y+1);
	lcd_ili9340c_pushColor(l, color);
	return (0);
}

#if 0
void Adafruit_ILI9340::drawFastVLine(int16_t x, int16_t y, int16_t h,
 uint16_t color) {

  // Rudimentary clipping
  if((x >= _width) || (y >= _height)) return;

  if((y+h-1) >= _height)
    h = _height-y;

  setAddrWindow(x, y, x, y+h-1);

  uint8_t hi = color >> 8, lo = color;

  SET_BIT(dcport, dcpinmask);
  //digitalWrite(_dc, HIGH);
  CLEAR_BIT(csport, cspinmask);
  //digitalWrite(_cs, LOW);

  while (h--) {
    spiwrite(hi);
    spiwrite(lo);
  }
  SET_BIT(csport, cspinmask);
  //digitalWrite(_cs, HIGH);
}


void Adafruit_ILI9340::drawFastHLine(int16_t x, int16_t y, int16_t w,
  uint16_t color) {

  // Rudimentary clipping
  if((x >= _width) || (y >= _height)) return;
  if((x+w-1) >= _width)  w = _width-x;
  setAddrWindow(x, y, x+w-1, y);

  uint8_t hi = color >> 8, lo = color;
  SET_BIT(dcport, dcpinmask);
  CLEAR_BIT(csport, cspinmask);
  //digitalWrite(_dc, HIGH);
  //digitalWrite(_cs, LOW);
  while (w--) {
    spiwrite(hi);
    spiwrite(lo);
  }
  SET_BIT(csport, cspinmask);
  //digitalWrite(_cs, HIGH);
}

void Adafruit_ILI9340::fillScreen(uint16_t color) {
  fillRect(0, 0,  _width, _height, color);
}

// fill a rectangle
void Adafruit_ILI9340::fillRect(int16_t x, int16_t y, int16_t w, int16_t h,
  uint16_t color) {

  // rudimentary clipping (drawChar w/big text requires this)
  if((x >= _width) || (y >= _height)) return;
  if((x + w - 1) >= _width)  w = _width  - x;
  if((y + h - 1) >= _height) h = _height - y;

  setAddrWindow(x, y, x+w-1, y+h-1);

  uint8_t hi = color >> 8, lo = color;

  SET_BIT(dcport, dcpinmask);
  //digitalWrite(_dc, HIGH);
  CLEAR_BIT(csport, cspinmask);
  //digitalWrite(_cs, LOW);

  for(y=h; y>0; y--) {
    for(x=w; x>0; x--) {
      spiwrite(hi);
      spiwrite(lo);
    }
  }
  //digitalWrite(_cs, HIGH);
  SET_BIT(csport, cspinmask);
}

#endif

static int
lcd_ili9340c_rowBlit(struct lcd *lcd, int16_t x, int16_t y,
    uint32_t *rgb, int l)
{
	struct lcd_ili9340c *h = lcd->hw;
	int i;
	uint16_t color;
	uint32_t c;

	/* Set starting point */
	lcd_ili9340c_setAddrWindow(h, x, y, lcd->tft_width, lcd->tft_height);

	/* Push lots of colours */
	for (i = 0; i < l; i++) {
		c = rgb[i];
		color = lcd_ili9340c_Color565((c >> 16) & 0xff,
		    (c >> 8) & 0xff,
		    (c) & 0xff);
		lcd_ili9340c_pushColor(h, color);
	}

	return (0);
}

static int
lcd_ili9340c_begin(struct lcd_ili9340c *l)
{

	// Toggle RST low to reset; CS low so it'll listen to us
	bit_clear(l, l->pin_cs);

	// toggle RST low to reset

	bit_set(l, l->pin_rst);
	usleep(5 * 1000);
	bit_clear(l, l->pin_rst);
	usleep(20 * 1000);
	bit_set(l, l->pin_rst);
	usleep(150 * 1000);

	lcd_ili9340c_writecommand(l, 0xEF);
	lcd_ili9340c_writedata(l, 0x03);
	lcd_ili9340c_writedata(l, 0x80);
	lcd_ili9340c_writedata(l, 0x02);

	lcd_ili9340c_writecommand(l, 0xCF);
	lcd_ili9340c_writedata(l, 0x00);
	lcd_ili9340c_writedata(l, 0XC1);
	lcd_ili9340c_writedata(l, 0X30);

	lcd_ili9340c_writecommand(l, 0xED);
	lcd_ili9340c_writedata(l, 0x64);
	lcd_ili9340c_writedata(l, 0x03);
	lcd_ili9340c_writedata(l, 0X12);
	lcd_ili9340c_writedata(l, 0X81);

	lcd_ili9340c_writecommand(l, 0xE8);
	lcd_ili9340c_writedata(l, 0x85);
	lcd_ili9340c_writedata(l, 0x00);
	lcd_ili9340c_writedata(l, 0x78);

	lcd_ili9340c_writecommand(l, 0xCB);
	lcd_ili9340c_writedata(l, 0x39);
	lcd_ili9340c_writedata(l, 0x2C);
	lcd_ili9340c_writedata(l, 0x00);
	lcd_ili9340c_writedata(l, 0x34);
	lcd_ili9340c_writedata(l, 0x02);

	lcd_ili9340c_writecommand(l, 0xF7);
	lcd_ili9340c_writedata(l, 0x20);

	lcd_ili9340c_writecommand(l, 0xEA);
	lcd_ili9340c_writedata(l, 0x00);
	lcd_ili9340c_writedata(l, 0x00);

	lcd_ili9340c_writecommand(l, ILI9340_PWCTR1);    //Power control
	lcd_ili9340c_writedata(l, 0x23);   //VRH[5:0]

	lcd_ili9340c_writecommand(l, ILI9340_PWCTR2);    //Power control
	lcd_ili9340c_writedata(l, 0x10);   //SAP[2:0];BT[3:0]

	lcd_ili9340c_writecommand(l, ILI9340_VMCTR1);    //VCM control
	lcd_ili9340c_writedata(l, 0x3e); //�Աȶȵ���
	lcd_ili9340c_writedata(l, 0x28);

	lcd_ili9340c_writecommand(l, ILI9340_VMCTR2);    //VCM control2
	lcd_ili9340c_writedata(l, 0x86);  //--

	lcd_ili9340c_writecommand(l, ILI9340_MADCTL);    // Memory Access Control
	/* 240x320 */
	//lcd_ili9340c_writedata(l, ILI9340_MADCTL_MX | ILI9340_MADCTL_BGR);
	/* 320x240 */
	lcd_ili9340c_writedata(l, ILI9340_MADCTL_MV | ILI9340_MADCTL_BGR);

	lcd_ili9340c_writecommand(l, ILI9340_PIXFMT);
	lcd_ili9340c_writedata(l, 0x55);

	lcd_ili9340c_writecommand(l, ILI9340_FRMCTR1);
	lcd_ili9340c_writedata(l, 0x00);
	lcd_ili9340c_writedata(l, 0x18);

	lcd_ili9340c_writecommand(l, ILI9340_DFUNCTR);    // Display Function Control
	lcd_ili9340c_writedata(l, 0x08);
	lcd_ili9340c_writedata(l, 0x82);
	lcd_ili9340c_writedata(l, 0x27);

	lcd_ili9340c_writecommand(l, 0xF2);    // 3Gamma Function Disable
	lcd_ili9340c_writedata(l, 0x00);

	lcd_ili9340c_writecommand(l, ILI9340_GAMMASET);    //Gamma curve selected
	lcd_ili9340c_writedata(l, 0x01);

	lcd_ili9340c_writecommand(l, ILI9340_GMCTRP1);    //Set Gamma
	lcd_ili9340c_writedata(l, 0x0F);
	lcd_ili9340c_writedata(l, 0x31);
	lcd_ili9340c_writedata(l, 0x2B);
	lcd_ili9340c_writedata(l, 0x0C);
	lcd_ili9340c_writedata(l, 0x0E);
	lcd_ili9340c_writedata(l, 0x08);
	lcd_ili9340c_writedata(l, 0x4E);
	lcd_ili9340c_writedata(l, 0xF1);
	lcd_ili9340c_writedata(l, 0x37);
	lcd_ili9340c_writedata(l, 0x07);
	lcd_ili9340c_writedata(l, 0x10);
	lcd_ili9340c_writedata(l, 0x03);
	lcd_ili9340c_writedata(l, 0x0E);
	lcd_ili9340c_writedata(l, 0x09);
	lcd_ili9340c_writedata(l, 0x00);

	lcd_ili9340c_writecommand(l, ILI9340_GMCTRN1);    //Set Gamma
	lcd_ili9340c_writedata(l, 0x00);
	lcd_ili9340c_writedata(l, 0x0E);
	lcd_ili9340c_writedata(l, 0x14);
	lcd_ili9340c_writedata(l, 0x03);
	lcd_ili9340c_writedata(l, 0x11);
	lcd_ili9340c_writedata(l, 0x07);
	lcd_ili9340c_writedata(l, 0x31);
	lcd_ili9340c_writedata(l, 0xC1);
	lcd_ili9340c_writedata(l, 0x48);
	lcd_ili9340c_writedata(l, 0x08);
	lcd_ili9340c_writedata(l, 0x0F);
	lcd_ili9340c_writedata(l, 0x0C);
	lcd_ili9340c_writedata(l, 0x31);
	lcd_ili9340c_writedata(l, 0x36);
	lcd_ili9340c_writedata(l, 0x0F);

	lcd_ili9340c_writecommand(l, ILI9340_SLPOUT);    //Exit Sleep
	usleep(120 * 1000);
	lcd_ili9340c_writecommand(l, ILI9340_DISPON);    //Display on

	return (0);
}

struct lcd *
lcd_ili9340c_init(struct lcd_ili9340c_cfg *cfg)
{
	struct lcd_ili9340c *h = 0;
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
	l->tft_width = 320;
	l->tft_height = 240;
	l->lcd_pixel = lcd_ili9340c_drawPixel;
//	l->lcd_hline = lcd_ili9340c_rawFastHLine;
//	l->lcd_vline = lcd_ili9340c_rawFastVLine;
	l->lcd_row_blit = lcd_ili9340c_rowBlit;

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
	lcd_ili9340c_begin(h);

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
