/*
TinyScreen.h - Last modified 28 April 2015

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

Written by Ben Rose for TinyCircuits.

The latest version of this library can be found at https://tiny-circuits.com/
*/

#include "Arduino.h"
// GPIO Pins
const uint8_t GPIO_DC =   0x01;
const uint8_t GPIO_CS =   0x02;
const uint8_t GPIO_SHDN = 0x04;
const uint8_t GPIO_RES =  0x08;
const uint8_t GPIO_BTN1 = 0x10;
const uint8_t GPIO_BTN2 = 0x20;
const uint8_t GPIO_BTN3 = 0x40;
const uint8_t GPIO_BTN4 = 0x80;
const uint8_t GPIO_CMD_START = ~(GPIO_CS|GPIO_DC);
const uint8_t GPIO_DATA_START = ~GPIO_CS;
const uint8_t GPIO_TRANSFER_END = GPIO_CS|GPIO_SHDN;

//GPIO Registers
const uint8_t GPIO_RegData = 0x00;
const uint8_t GPIO_RegDir = 0x01;
const uint8_t GPIO_RegPullUp = 0x02;

const uint8_t GPIO_ADDR = 0x20;

typedef struct
{
	const uint8_t width;
	const uint16_t offset;
	
} FONT_CHAR_INFO;	

typedef struct
{
	const unsigned char height;
	const char startCh;
	const char endCh;
	const FONT_CHAR_INFO*	charDesc;
	const unsigned char* bitmap;
		
} FONT_INFO;	

#include <avr/pgmspace.h>
#include "font.h"

class TinyScreen : public Print {
 public:
 //init, control
  TinyScreen(uint8_t);
  void startData(void);
  void startCommand(void);
  void endTransfer(void);
  void begin(void);
  void on(void);
  void off(void);
  void setFlip(uint8_t);
  void setMirror(uint8_t);
  void setBitDepth(uint8_t);
  void setBrightness(uint8_t);
  void writeRemap(void);
  //accelerated drawing commands
  void drawPixel(uint8_t, uint8_t, uint16_t);
  void drawLine(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);
  void drawLine(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);
  void drawRect(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);
  void drawRect(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);
  void clearWindow(uint8_t, uint8_t, uint8_t, uint8_t);
  //basic graphics commands
  void writePixel(uint16_t);
  void writeBuffer(uint8_t *, int);
  void setX(uint8_t, uint8_t);
  void setY(uint8_t, uint8_t);
  void goTo(uint8_t x, uint8_t y);
  //I2C GPIO related
  uint8_t getButtons(void);
  void writeGPIO(uint8_t, uint8_t);
  //font
  void setFont(const FONT_INFO&);
  void setCursor(uint8_t, uint8_t);
  void fontColor(uint8_t, uint8_t);
  virtual size_t write(uint8_t);
  
  static const uint8_t xMax=95;
  static const uint8_t yMax=63;
 private:
  
  uint8_t _addr, _cursorX, _cursorY, _fontHeight, _fontFirstCh, _fontLastCh, _fontColor, _fontBGcolor, _bitDepth, _flipDisplay, _mirrorDisplay;
  const FONT_CHAR_INFO* _fontDescriptor;
  const unsigned char* _fontBitmap;
};
