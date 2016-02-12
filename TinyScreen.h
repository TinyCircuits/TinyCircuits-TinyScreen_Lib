/*
TinyScreen.h - Last modified 11 February 2016

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

#ifndef TinyScreen_h
#define TinyScreen_h

#include "Arduino.h"
#include "SPI.h"

// Color definitions
// 8b: BBBG GGRR
const uint8_t TS_8b_Black     = 0x00;
const uint8_t TS_8b_Gray      = 0x6D;
const uint8_t TS_8b_White     = 0xFF;
const uint8_t TS_8b_Blue      = 0xE0;
const uint8_t TS_8b_DarkBlue  = 0x60;
const uint8_t TS_8b_Red       = 0x03;
const uint8_t TS_8b_DarkRed   = 0x01;
const uint8_t TS_8b_Green     = 0x1C;
const uint8_t TS_8b_DarkGreen = 0x0C;
const uint8_t TS_8b_Brown     = 0x32;
const uint8_t TS_8b_DarkBrown = 0x22;
const uint8_t TS_8b_Yellow    = 0x1F;

// 16b: BBBB BGGG GGGR RRRR
const uint16_t TS_16b_Black     = 0x0000;
const uint16_t TS_16b_Gray      = 0x7BEF;
const uint16_t TS_16b_DarkGray  = 0x39E7;
const uint16_t TS_16b_White     = 0xFFFF;
const uint16_t TS_16b_Blue      = 0xF800;
const uint16_t TS_16b_DarkBlue  = 0x7800;
const uint16_t TS_16b_Red       = 0x001F;
const uint16_t TS_16b_DarkRed   = 0x000F;
const uint16_t TS_16b_Green     = 0x07E0;
const uint16_t TS_16b_DarkGreen = 0x03E0;
const uint16_t TS_16b_Brown     = 0x0C10;
const uint16_t TS_16b_DarkBrown = 0x0810;
const uint16_t TS_16b_Yellow    = 0x07FF;

// TinyScreen types
const uint8_t TinyScreenDefault   = 0;
const uint8_t TinyScreenAlternate = 1;
const uint8_t TinyScreenPlus      = 2;

// TinyScreen Rectangle Fills
const uint8_t TSRectangleFilled = 1;
const uint8_t TSRectangleNoFill = 0;

// TinyScreen bitDepths
const uint8_t TSBitDepth8  = 0;
const uint8_t TSBitDepth16 = 1;

// TinyScreen Color Modes
const uint8_t TSColorModeBGR = 0;
const uint8_t TSColorModeRGB = 1;

// TinyScreen button definitions
const uint8_t TSButtonUpperLeft  = 1<<1;
const uint8_t TSButtonUpperRight = 1<<2;
const uint8_t TSButtonLowerLeft  = 1<<0;
const uint8_t TSButtonLowerRight = 1<<3;

// TinyScreen+ pin defintions
const uint8_t TSP_PIN_DC   = 22;
const uint8_t TSP_PIN_CS   = 38;
const uint8_t TSP_PIN_SHDN = 27;
const uint8_t TSP_PIN_RST  = 26;

const uint8_t TSP_PIN_BT1  = 19;
const uint8_t TSP_PIN_BT2  = 25;
const uint8_t TSP_PIN_BT3  = 30;
const uint8_t TSP_PIN_BT4  = 31;

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
const uint8_t GPIO_RegInterruptMask = 0x05;
const uint8_t GPIO_RegSenseHigh = 0x06;
const uint8_t GPIO_RegInterruptSource = 0x08;

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

//#include <avr/pgmspace.h>
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
  void setColorMode(uint8_t);
  void setBrightness(uint8_t);
  void writeRemap(void);
  //accelerated drawing commands
  void drawPixel(uint8_t, uint8_t, uint16_t);
  void drawLine(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);
  void drawLine(uint8_t, uint8_t, uint8_t, uint8_t, uint16_t);
  void drawRect(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);
  void drawRect(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint16_t);
  void clearWindow(uint8_t, uint8_t, uint8_t, uint8_t);
  void clearScreen(void);
  //basic graphics commands
  void writePixel(uint16_t);
  void writeBuffer(uint8_t *, int);
  void setX(uint8_t, uint8_t);
  void setY(uint8_t, uint8_t);
  void goTo(uint8_t x, uint8_t y);
  //I2C GPIO related
  uint8_t getButtons(uint8_t);
  uint8_t getButtons(void);
  void writeGPIO(uint8_t, uint8_t);
  //font
  void setFont(const FONT_INFO&);
  uint8_t getFontHeight(const FONT_INFO&);
  uint8_t getFontHeight(void);
  uint8_t getPrintWidth(char *);
  void setCursor(uint8_t, uint8_t);
  void fontColor(uint16_t, uint16_t);
  virtual size_t write(uint8_t);
  //DMA for SAMD
  void initDMA(void);
  uint8_t getReadyStatusDMA(void);
  void writeBufferDMA(uint8_t *,int);
  
  static const uint8_t xMax=95;
  static const uint8_t yMax=63;
 private:
  
  uint8_t _addr, _cursorX, _cursorY, _fontHeight, _fontFirstCh, _fontLastCh, _bitDepth, _flipDisplay, _mirrorDisplay, _colorMode, _externalIO, _type;
  uint16_t  _fontColor, _fontBGcolor;
  const FONT_CHAR_INFO* _fontDescriptor;
  const unsigned char* _fontBitmap;
  SPIClass *TSSPI;
};

#endif