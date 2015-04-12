FreeBSD LCD GPIO
----------------

This is a simple example LCD drawing library for use with FreeBSD and
libgpio.  The supported boards are:

* Adafruit 96x64 colour OLED: http://www.adafruit.com/products/684

This has been tested on the Carambola 2 MIPS platform (compiled natively,
but cross-compiling should be easy enough.)  It uses libgpio to access
the GPIO pins.

Documentation on connecting up the LCD module to a Carambola 2 evaluation
board can be found in the doc/ directory.
