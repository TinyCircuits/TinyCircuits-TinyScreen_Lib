# TinyCircuits TinyScreen/TinyScreen+ Arduino Library

This library allows for easy use of the TinyScreen display and input buttons. Text, shapes, pixel manipulation, flipping, mirroring, and more allow you to display data, pictures, video, games or whatever you can fit in 96 by 64 pixels.

## Basic Example

An example demonstrating and explaining most of the library functions is included. A list of further notes:

* Include the TinyScreen, SPI, and Wire libraries (Wire can be omitted from TinyScreen+ code if necessary)
* Declare TinyScreen as display, and use the correct board type(TinyScreenDefault, TinyScreenAlternate, TinyScreenPlus): TinyScreen display = TinyScreen(TinyScreenPlus);
* TinyScreen library defaults to BGR colors, and this is what the TS_8b and TS_16b color definitions use. This can be changed with setColorMode(TSColorModeRGB);
* Testing for a button press can now be done in a readable way, and works the same when the display is flipped: if (display.getButtons(TSButtonUpperLeft)) { };
* TinyScreen+ supports DMA data transfers- check the end of TinySCreen.cpp