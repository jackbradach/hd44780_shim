hd44780_shim
A SPI/I2C to hd44780 shim, running on an ATTiny84 (or similar).

This is an in-progress weekend project I started to solve the
problem of wanting to use a character LCD but not wanting to
use 6+ pins doing so.  I could have use an MCP23018/MCP23S18
(I2C/SPI GPIO expanders), but the ATTiny84 is about a buck more
than either chip and gives me more flexibility.

The schematic (which I'll upload when I either scan it or redraw
in KiCad) isn't anything too clever;  PORTA[3:0] -> HD44780[7:4],
PORTB[1:0] -> HD44780[6,4], with the USI (Universal Serial Interface)
pinned out to be connected to a master.  I was intending for this
to just be a SPI or I2C controller, but anything the USI supports I'll
probably implement eventually.

Of note is that multiple LCDs can be supported simultaneously.
Each LCD can make use of a different backend for the hardware interface;
AVR GPIO, I/O expanders, etc.  I've code I wrote for an MCP23018
to merge in, although that probably won't be of interest on an ATTiny!
I haven't done too much optimization yet (since it meets my needs
currently);  the LCD timing could probably be a little tighter
as could memory usage.

A demo showing a wipe transition may be viewed at:
https://youtu.be/uWoHyUY6dOQ.

Feel free to reuse / adapt any parts of this you find useful, although if
it's a direct cut-and-paste, drop a note where you swiped it from.  I'd
also be interested to hear if you found this useful!
