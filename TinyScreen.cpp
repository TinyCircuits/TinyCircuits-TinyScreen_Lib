/*
TinyScreen.cpp - Last modified 11 February 2016

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

#include "TinyScreen.h"
#include <avr/pgmspace.h>
#include "pins_arduino.h"
#include "wiring_private.h"
#include <SPI.h>
#include <Wire.h>

//These delays are used to allow hardware drawing commands on the SSD1331 to execute
#ifndef TS_USE_DELAY
#define TS_USE_DELAY true
#endif

/*
SPI optimization defines for known architectures
*/


#if defined(ARDUINO_ARCH_AVR)
  #define TS_SPI_SET_DATA_REG(x) SPDR=(x)
#elif defined(ARDUINO_ARCH_SAMD)
  #define TS_SPI_SET_DATA_REG(x) if(_externalIO){SERCOM1->SPI.DATA.bit.DATA=(x);}else{SERCOM4->SPI.DATA.bit.DATA=(x);}
#elif defined(ARDUINO_ARCH_ESP8266)
  #define TS_SPI_SET_DATA_REG(x) SPI1W0 = (x); SPI1CMD |= SPIBUSY
#else
  #define TS_SPI_SET_DATA_REG(x) TSSPI->transfer(x)
#endif

#if defined(ARDUINO_ARCH_AVR)
  #define TS_SPI_SEND_WAIT() while(!(SPSR & _BV(SPIF)))
#elif defined(ARDUINO_ARCH_SAMD)
  #define TS_SPI_SEND_WAIT() if(_externalIO){while(SERCOM1->SPI.INTFLAG.bit.DRE == 0);}else{while(SERCOM4->SPI.INTFLAG.bit.DRE == 0);}
#elif defined(ARDUINO_ARCH_ESP8266)
  #define TS_SPI_SEND_WAIT() while(SPI1CMD & SPIBUSY)
#else
  #define TS_SPI_SEND_WAIT() if(0)
#endif


/*
TinyScreen uses an I2C GPIO chip to interface with the OLED control lines and buttons
TinyScreen+ has direct IO and uses the Arduino digital IO interface
writeGPIO(address, data);//write to SX1505
startCommand();//write SSD1331 chip select active with data/command signalling a command
startData();//write SSD1331 chip select active with data/command signalling data
endTransfer();//write SSD1331 chip select inactive
getButtons();//read button states, return as four LSBs in a byte- optional button mask
*/

void TinyScreen::writeGPIO(uint8_t regAddr, uint8_t regData)
{
#if defined(ARDUINO_ARCH_AVR)
  uint8_t oldTWBR=TWBR;
  TWBR=0;
#endif
  Wire.beginTransmission(GPIO_ADDR+_addr);
  Wire.write(regAddr); 
  Wire.write(regData);
  Wire.endTransmission();
#if defined(ARDUINO_ARCH_AVR)
  TWBR=oldTWBR;
#endif
}

void TinyScreen::startCommand(void) {
  if(_externalIO){
    writeGPIO(GPIO_RegData,GPIO_CMD_START);
  }else{
    digitalWrite(TSP_PIN_DC,LOW);
    digitalWrite(TSP_PIN_CS,LOW);
  }
}

void TinyScreen::startData(void) {
  if(_externalIO){
    writeGPIO(GPIO_RegData,GPIO_DATA_START);
  }else{
    digitalWrite(TSP_PIN_DC,HIGH);
    digitalWrite(TSP_PIN_CS,LOW);
  }
}

void TinyScreen::endTransfer(void) {
  if(_externalIO){
    writeGPIO(GPIO_RegData,GPIO_TRANSFER_END);
  }else{
    digitalWrite(TSP_PIN_CS,HIGH);
  }
}

uint8_t TinyScreen::getButtons(uint8_t buttonMask) {
  uint8_t buttons=0;
  if(_externalIO){
    Wire.beginTransmission(GPIO_ADDR+_addr);
    Wire.write(GPIO_RegData);
    Wire.endTransmission();
    Wire.requestFrom(GPIO_ADDR+_addr,1);
    buttons=Wire.read();
    //buttons are active low and MSBs, so flip and shift
    buttons=((~buttons)>>4)&0x0F;
  }else{
    if(!digitalRead(TSP_PIN_BT1))buttons|=0x01;
    if(!digitalRead(TSP_PIN_BT2))buttons|=0x02;
    if(!digitalRead(TSP_PIN_BT3))buttons|=0x04;
    if(!digitalRead(TSP_PIN_BT4))buttons|=0x08;
  }
  if(_flipDisplay){
    uint8_t flipped=0;
    flipped|=((buttons&TSButtonUpperLeft)<<2);
    flipped|=((buttons&TSButtonUpperRight)>>2);
    flipped|=((buttons&TSButtonLowerLeft)<<2);
    flipped|=((buttons&TSButtonLowerRight)>>2);
    buttons=flipped;
  }
  return buttons&buttonMask;
}

uint8_t TinyScreen::getButtons(void) {
  return getButtons(TSButtonUpperLeft|TSButtonUpperRight|TSButtonLowerLeft|TSButtonLowerRight);
}

/*
SSD1331 Basics
goTo(x,y);//set OLED RAM to pixel address (x,y) with wrap around at x and y max
setX(x start, x end);//set OLED RAM to x start, wrap around at x end
setY(y start, y end);//set OLED RAM to y start, wrap around at y end
*/

void TinyScreen::goTo(uint8_t x, uint8_t y) {
  if(x>xMax||y>yMax)return;
  setX(x,xMax);
  setY(y,yMax);
}

void TinyScreen::setX(uint8_t x, uint8_t end) {
  if(x>xMax)x=xMax;
  if(end>xMax)end=xMax;
  startCommand();
  TSSPI->transfer(0x15);//set column
  TSSPI->transfer(x);
  TSSPI->transfer(end);
  endTransfer();
}

void TinyScreen::setY(uint8_t y, uint8_t end) {
  if(y>yMax)y=yMax;
  if(end>yMax)end=yMax;
  startCommand();
  TSSPI->transfer(0x75);//set row
  TSSPI->transfer(y);
  TSSPI->transfer(end);
  endTransfer();
}

/*
Hardware accelerated drawing functions:
clearWindow(x start, y start, width, height);//clears specified OLED controller memory
clearScreen();//clears entire screen
drawRect(x stary, y start, width, height, fill, 8bitcolor);//sets specified OLED controller memory to an 8 bit color, fill is a boolean
drawRect(x stary, y start, width, height, fill, 16bitcolor);//sets specified OLED controller memory to an 8 bit color, fill is a boolean
drawRect(x stary, y start, width, height, fill, red, green, blue);//like above, but uses 6 bit color values. Red and blue ignore the LSB.
drawLine(x1, y1, x2, y2, 8bitcolor);//draw a line from (x1,y1) to (x2,y2) with an 8 bit color
drawLine(x1, y1, x2, y2, 16bitcolor);//draw a line from (x1,y1) to (x2,y2) with an 16 bit color
drawLine(x1, y1, x2, y2, red, green, blue);//like above, but uses 6 bit color values. Red and blue ignore the LSB.
*/

void TinyScreen::clearWindow(uint8_t x, uint8_t y, uint8_t w, uint8_t h) {  
  if(x>xMax||y>yMax)return;
  uint8_t x2=x+w-1;
  uint8_t y2=y+h-1;
  if(x2>xMax)x2=xMax;
  if(y2>yMax)y2=yMax;
  
  startCommand();
  TSSPI->transfer(0x25);//clear window
  TSSPI->transfer(x);TSSPI->transfer(y);
  TSSPI->transfer(x2);TSSPI->transfer(y2);
  endTransfer();
#if TS_USE_DELAY
  delayMicroseconds(400);
#endif
}

void TinyScreen::clearScreen(){
  clearWindow(0,0,96,64);
}

void TinyScreen::drawRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t f, uint8_t r, uint8_t g, uint8_t b) 
{
  if(x>xMax||y>yMax)return;
  uint8_t x2=x+w-1;
  uint8_t y2=y+h-1;
  if(x2>xMax)x2=xMax;
  if(y2>yMax)y2=yMax;
  
  uint8_t fill=0;
  if(f)fill=1;
  
  startCommand();
  TSSPI->transfer(0x26);//set fill
  TSSPI->transfer(fill);
  
  TSSPI->transfer(0x22);//draw rectangle
  TSSPI->transfer(x);TSSPI->transfer(y);
  TSSPI->transfer(x2);TSSPI->transfer(y2);
  //outline
  TSSPI->transfer(b);TSSPI->transfer(g);TSSPI->transfer(r);
  //fill
  TSSPI->transfer(b);TSSPI->transfer(g);TSSPI->transfer(r);
  endTransfer();
#if TS_USE_DELAY
  delayMicroseconds(400);
#endif
}

void TinyScreen::drawRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t f, uint16_t color) 
{
  uint16_t r,g,b;
  if(_bitDepth){
    r=(color)&0x1F;//five bits
    g=(color>>5)&0x3F;//six bits
    b=(color>>11)&0x1F;//five bits
    r=r<<1;//shift to fill six bits
    g=g<<0;//shift to fill six bits
    b=b<<1;//shift to fill six bits
  }else{
    r=(color)&0x03;//two bits
    g=(color>>2)&0x07;//three bits
    b=(color>>5)&0x07;//three bits
    r|=(r<<4)|(r<<2);//copy to fill six bits
    g|=g<<3;//copy to fill six bits
    b|=b<<3;//copy to fill six bits
  }
  drawRect(x,y,w,h,f,r,g,b);
}

void TinyScreen::drawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t r, uint8_t g, uint8_t b) {  
  if(x0>xMax)x0=xMax;
  if(y0>yMax)y0=yMax;
  if(x1>xMax)x1=xMax;
  if(y1>yMax)y1=yMax;
  startCommand();
  TSSPI->transfer(0x21);//draw line
  TSSPI->transfer(x0);TSSPI->transfer(y0);
  TSSPI->transfer(x1);TSSPI->transfer(y1);
  TSSPI->transfer(b);TSSPI->transfer(g);TSSPI->transfer(r);
  endTransfer();
#if TS_USE_DELAY
  delayMicroseconds(100);
#endif
}

void TinyScreen::drawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint16_t color) {
  uint16_t r,g,b;
  if(_bitDepth){
    r=(color)&0x1F;//five bits
    g=(color>>5)&0x3F;//six bits
    b=(color>>11)&0x1F;//five bits
    r=r<<1;//shift to fill six bits
    g=g<<0;//shift to fill six bits
    b=b<<1;//shift to fill six bits
  }else{
    r=(color)&0x03;//two bits
    g=(color>>2)&0x07;//three bits
    b=(color>>5)&0x07;//three bits
    r|=(r<<4)|(r<<2);//copy to fill six bits
    g|=g<<3;//copy to fill six bits
    b|=b<<3;//copy to fill six bits
  }
  drawLine(x0,y0,x1,y1,r,g,b);
}

/*
Pixel manipulation
drawPixel(x,y,color);//set pixel (x,y) to specified color. This is slow because we need to send commands setting the x and y, then send the pixel data.
writePixel(color);//write the current pixel to specified color. Less slow than drawPixel, but still has to ready display for pixel data
writeBuffer(buffer,count);//optimized write of a large buffer of 8 bit data. Must be wrapped with startData() and endTransfer(), but there can be any amount of calls to writeBuffer between.
*/

void TinyScreen::drawPixel(uint8_t x, uint8_t y, uint16_t color)
{
  if(x>xMax||y>yMax)return;
  goTo(x,y);
  writePixel(color);
}

void TinyScreen::writePixel(uint16_t color) {
  startData();
  if(_bitDepth)
    TSSPI->transfer(color>>8);
  TSSPI->transfer(color);
  endTransfer();
}

void TinyScreen::writeBuffer(uint8_t *buffer,int count) {
  uint8_t temp;
  TS_SPI_SET_DATA_REG(buffer[0]);
  for(int j=1;j<count;j++){
    temp=buffer[j];
    TS_SPI_SEND_WAIT();
    TS_SPI_SET_DATA_REG(temp);
  }
  TS_SPI_SEND_WAIT();
}

/* 
TinyScreen commands
setBrightness(brightness);//sets main current level, valid levels are 0-15
on();//turns display on
off();//turns display off, uses less power
setBitDepth(depth);//boolean- 0 is 8 bit, 1 is 16 bit
setFlip(flip);//done in hardware on the SSD1331. boolean- 0 is normal, 1 is upside down
setMirror(mirror);//done in hardware on the SSD1331. boolean- 0 is normal, 1 is mirrored across Y axis
*/

void TinyScreen::setBrightness(uint8_t brightness) {
  if(brightness>15)brightness=15;  
  startCommand();
  TSSPI->transfer(0x87);//set master current
  TSSPI->transfer(brightness);
  endTransfer();
}

void TinyScreen::on(void) {
  if(!_externalIO){
    digitalWrite(TSP_PIN_SHDN,HIGH);
  }
  startCommand();//if _externalIO, this will turn boost converter on
  delayMicroseconds(10000);
  TSSPI->transfer(0xAF);//display on
  endTransfer();
}

void TinyScreen::off(void) {
  startCommand();
  TSSPI->transfer(0xAE);//display off
  endTransfer();
  if(_externalIO){
    writeGPIO(GPIO_RegData,~GPIO_SHDN);//bost converter off
    //any other write will turn the boost converter back on
  }else{
    digitalWrite(TSP_PIN_SHDN,LOW);//SHDN
  }
}

void TinyScreen::setBitDepth(uint8_t b){
  _bitDepth=b;
  writeRemap();
}

void TinyScreen::setFlip(uint8_t f){
  _flipDisplay=f;
  writeRemap();
}

void TinyScreen::setMirror(uint8_t m){
  _mirrorDisplay=m;
  writeRemap();
}

void TinyScreen::setColorMode(uint8_t cm){
  _colorMode=cm;
  writeRemap();
}

/*
The SSD1331 remap command sets a lot of driver variables, these are kept in memory
and are all written when a change is made.
*/

void TinyScreen::writeRemap(void){
  uint8_t remap=(1<<5)|(1<<2);
  if(_flipDisplay)
    remap|=((1<<4)|(1<<1));
  if(_mirrorDisplay)
    remap^=(1<<1);
  if(_bitDepth)
    remap|=(1<<6);
  if(_colorMode)
    remap^=(1<<2);
  startCommand();
  TSSPI->transfer(0xA0);//set remap
  TSSPI->transfer(remap);
  endTransfer();
}

void TinyScreen::begin(void) {
  //init SPI
  TSSPI->begin();
  TSSPI->setDataMode(SPI_MODE0);//wrong mode, works because we're only writing. this mode is compatible with SD cards.
#if defined(ARDUINO_ARCH_AVR)
  TSSPI->setClockDivider(SPI_CLOCK_DIV2);
#elif defined(ARDUINO_ARCH_SAMD)
  TSSPI->setClockDivider(4);
#endif

  if(_externalIO){
    //standard TinyScreen- setup GPIO, reset SSD1331
    writeGPIO(GPIO_RegData,~GPIO_RES);//reset low, other pins high
    writeGPIO(GPIO_RegDir,~GPIO_RES);//set reset to output
    delay(5);
    writeGPIO(GPIO_RegDir,~(GPIO_CS|GPIO_DC|GPIO_SHDN));//reset to input, CS/DC/SHDN output
    writeGPIO(GPIO_RegPullUp,GPIO_BTN1|GPIO_BTN2|GPIO_BTN3|GPIO_BTN4);//button pullup enable
  }else{
    //otherwise TinyScreen+, connected directly to IO pins
    pinMode(TSP_PIN_SHDN,OUTPUT);pinMode(TSP_PIN_DC,OUTPUT);pinMode(TSP_PIN_CS,OUTPUT);pinMode(TSP_PIN_RST,OUTPUT);
    digitalWrite(TSP_PIN_SHDN,LOW);digitalWrite(TSP_PIN_DC,HIGH);digitalWrite(TSP_PIN_CS,HIGH);digitalWrite(TSP_PIN_RST,HIGH);
    pinMode(TSP_PIN_BT1,INPUT_PULLUP);pinMode(TSP_PIN_BT2,INPUT_PULLUP);pinMode(TSP_PIN_BT3,INPUT_PULLUP);pinMode(TSP_PIN_BT4,INPUT_PULLUP);
    //reset
    digitalWrite(TSP_PIN_RST,LOW);
    delay(5);
    digitalWrite(TSP_PIN_RST,HIGH);
  }
  delay(10);
  
  //datasheet SSD1331 init sequence
  const uint8_t init[32]={0xAE, 0xA1, 0x00, 0xA2, 0x00, 0xA4, 0xA8, 0x3F,
  0xAD, 0x8E, 0xB0, 0x0B, 0xB1, 0x31, 0xB3, 0xF0, 0x8A, 0x64, 0x8B,
  0x78, 0x8C, 0x64, 0xBB, 0x3A, 0xBE, 0x3E, 0x81, 0x91, 0x82, 0x50, 0x83, 0x7D};
  off();
  startCommand();
  for(uint8_t i=0;i<32;i++)
    TSSPI->transfer(init[i]);
  endTransfer();
  //use libarary functions for remaining init
  setBrightness(5);
  writeRemap();
  clearWindow(0,0,96,64);
  on();
}

/*
TinyScreen constructor
type tells us if we're using a regular TinyScreen, alternate addresss TinyScreen, or a TinyScreen+
address sets I2C address of SX1505 to 0x20 or 0x21, which is set by the position of a resistor near SX1505 (see schematic and board design)
*/

TinyScreen::TinyScreen(uint8_t type){
  _externalIO=0;
  _cursorX=0;
  _cursorY=0;
  _fontHeight=0;
  _fontFirstCh=0;
  _fontLastCh=0;
  _fontDescriptor=0;
  _fontBitmap=0;
  _fontColor=0xFFFF;
  _fontBGcolor=0x0000;
  _bitDepth=0;
  _flipDisplay=0;
  _mirrorDisplay=0;
  _colorMode=0;
  _type=type;
  
  //type determines the SPI interface IO configuration
  if(_type==TinyScreenDefault){
    TSSPI=&SPI;
    _externalIO=1;
    _addr=0;
  }else if(_type==TinyScreenAlternate){
    TSSPI=&SPI;
    _externalIO=1;
    _addr=1;
  }else if(_type==TinyScreenPlus){
#if defined(ARDUINO_ARCH_SAMD)
    TSSPI=&SPI1;
#endif
    _externalIO=0;
  }else{
    TSSPI=&SPI;
    _externalIO=1;
    _addr=0;
  }
}

/*
TinyScreen Text Display
setCursor(x,y);//set text cursor position to (x,y)
setFont(descriptor);//set font data to use
fontColor(text color, background color);//sets text and background color
getFontHeight();//returns height of font

getStringWidth
*/

void TinyScreen::setCursor(uint8_t x, uint8_t y){
  _cursorX=x;
  _cursorY=y;
}

void TinyScreen::setFont(const FONT_INFO& fontInfo){
  _fontHeight=fontInfo.height;
  _fontFirstCh=fontInfo.startCh;
  _fontLastCh=fontInfo.endCh;
  _fontDescriptor=fontInfo.charDesc;
  _fontBitmap=fontInfo.bitmap;
}

void TinyScreen::fontColor(uint16_t f, uint16_t g){
  _fontColor=f;
  _fontBGcolor=g;
}

uint8_t TinyScreen::getFontHeight(const FONT_INFO& fontInfo){
  return fontInfo.height;
}

uint8_t TinyScreen::getFontHeight(){
  return _fontHeight;
}

uint8_t TinyScreen::getPrintWidth(char * st){
  if(!_fontFirstCh)return 0;
  uint8_t i,amtCh,totalWidth;
  totalWidth=0;
  amtCh=strlen(st);
  for(i=0;i<amtCh;i++){
    totalWidth+=pgm_read_byte(&_fontDescriptor[st[i]-_fontFirstCh].width)+1;
  }
  return totalWidth;
}

size_t TinyScreen::write(uint8_t ch){
  if(!_fontFirstCh)return 1;
  if(ch<_fontFirstCh || ch>_fontLastCh)return 1;
  if(_cursorX>xMax || _cursorY>yMax)return 1;
  uint8_t chWidth=pgm_read_byte(&_fontDescriptor[ch-_fontFirstCh].width);
  uint8_t bytesPerRow=chWidth/8;
  if(chWidth>bytesPerRow*8)
    bytesPerRow++;
  uint16_t offset=pgm_read_word(&_fontDescriptor[ch-_fontFirstCh].offset)+(bytesPerRow*_fontHeight)-1;
  
  setX(_cursorX,_cursorX+chWidth+1);
  setY(_cursorY,_cursorY+_fontHeight);
  
  startData();
  for(uint8_t y=0; y<_fontHeight && y+_cursorY<yMax+1; y++){
    if(_bitDepth){
      TS_SPI_SET_DATA_REG(_fontBGcolor>>8);
      TS_SPI_SEND_WAIT();
    }
    TS_SPI_SET_DATA_REG(_fontBGcolor);
    for(uint8_t byte=0; byte<bytesPerRow; byte++){
      uint8_t data=pgm_read_byte(_fontBitmap+offset-y-((bytesPerRow-byte-1)*_fontHeight));
      uint8_t bits=byte*8;
        for(uint8_t i=0; i<8 && (bits+i)<chWidth && (bits+i+_cursorX)<xMax; i++){
          TS_SPI_SEND_WAIT();
          if(data&(0x80>>i)){
            if(_bitDepth){
              TS_SPI_SET_DATA_REG(_fontColor>>8);
              TS_SPI_SEND_WAIT();
            }
            TS_SPI_SET_DATA_REG(_fontColor);
           }else{
            if(_bitDepth){
              TS_SPI_SET_DATA_REG(_fontBGcolor>>8);
              TS_SPI_SEND_WAIT();
            }
            TS_SPI_SET_DATA_REG(_fontBGcolor);
          }
      }
    }
    TS_SPI_SEND_WAIT();
    if((_cursorX+chWidth)<xMax){
      if(_bitDepth){
        TS_SPI_SET_DATA_REG(_fontBGcolor>>8);
        TS_SPI_SEND_WAIT();
      }
      TS_SPI_SET_DATA_REG(_fontBGcolor);
      TS_SPI_SEND_WAIT();
    }
  }
  endTransfer();
  _cursorX+=(chWidth+1);
  return 1;
}

/*
TinyScreen+ SAMD21 DMA write
Example code taken from https://github.com/manitou48/ZERO/blob/master/SPIdma.ino
Thanks manitou!
This code is experimental
*/


#if defined(ARDUINO_ARCH_SAMD)

typedef struct {
  uint16_t btctrl;
  uint16_t btcnt;
  uint32_t srcaddr;
  uint32_t dstaddr;
  uint32_t descaddr;
} dmacdescriptor ;
volatile dmacdescriptor wrb[12] __attribute__ ((aligned (16)));
dmacdescriptor descriptor_section[12] __attribute__ ((aligned (16)));
dmacdescriptor descriptor __attribute__ ((aligned (16)));

const uint32_t DMAchannel = 0;
volatile uint32_t dmaReady=true;


void DMAC_Handler() {
  // interrupts DMAC_CHINTENCLR_TERR DMAC_CHINTENCLR_TCMPL DMAC_CHINTENCLR_SUSP
  uint8_t active_channel;

  // disable irqs ?
  __disable_irq();
  active_channel =  DMAC->INTPEND.reg & DMAC_INTPEND_ID_Msk; // get channel number
  DMAC->CHID.reg = DMAC_CHID_ID(active_channel);
  if(DMAC->CHINTFLAG.reg)dmaReady=true;
  DMAC->CHINTFLAG.reg = DMAC_CHINTENCLR_TCMPL; // clear
  DMAC->CHINTFLAG.reg = DMAC_CHINTENCLR_TERR;
  DMAC->CHINTFLAG.reg = DMAC_CHINTENCLR_SUSP;
  __enable_irq();
}

#endif

uint8_t TinyScreen::getReadyStatusDMA(){
#if defined(ARDUINO_ARCH_SAMD)
  return dmaReady;
#else
  //it's tough to raise an error about not having DMA in the IDE- try to fall back to regular software transfer
  return 1;//always return ready
#endif
}

void TinyScreen::writeBufferDMA(uint8_t *txdata,int n) {
#if defined(ARDUINO_ARCH_SAMD)
  while(!dmaReady);
  
  uint32_t temp_CHCTRLB_reg;
  // set up transmit channel  
  DMAC->CHID.reg = DMAC_CHID_ID(DMAchannel); 
  DMAC->CHCTRLA.reg &= ~DMAC_CHCTRLA_ENABLE;
  DMAC->CHCTRLA.reg = DMAC_CHCTRLA_SWRST;
  DMAC->SWTRIGCTRL.reg &= (uint32_t)(~(1 << DMAchannel));
  if(_externalIO){
    temp_CHCTRLB_reg = DMAC_CHCTRLB_LVL(0) | DMAC_CHCTRLB_TRIGSRC(SERCOM1_DMAC_ID_TX) | DMAC_CHCTRLB_TRIGACT_BEAT;
  }else{
    temp_CHCTRLB_reg = DMAC_CHCTRLB_LVL(0) | DMAC_CHCTRLB_TRIGSRC(SERCOM4_DMAC_ID_TX) | DMAC_CHCTRLB_TRIGACT_BEAT;
  }
  DMAC->CHCTRLB.reg = temp_CHCTRLB_reg;
  DMAC->CHINTENSET.reg = DMAC_CHINTENSET_MASK ; // enable interrupts
  descriptor.descaddr = 0;
  if(_externalIO){
    descriptor.dstaddr = (uint32_t) &SERCOM1->SPI.DATA.reg;
  }else{
    descriptor.dstaddr = (uint32_t) &SERCOM4->SPI.DATA.reg;
  }
  descriptor.btcnt =  n;
  descriptor.srcaddr = (uint32_t)txdata;
  descriptor.btctrl =  DMAC_BTCTRL_VALID;
  descriptor.srcaddr += n;
  descriptor.btctrl |= DMAC_BTCTRL_SRCINC;
  memcpy(&descriptor_section[DMAchannel],&descriptor, sizeof(dmacdescriptor));

  dmaReady = false;
  
  // start channel
  DMAC->CHID.reg = DMAC_CHID_ID(DMAchannel);
  DMAC->CHCTRLA.reg |= DMAC_CHCTRLA_ENABLE;
  
  //DMAC->CHID.reg = DMAC_CHID_ID(chnltx);   //disable DMA to allow lib SPI- necessary? needs to be done after completion
  //DMAC->CHCTRLA.reg &= ~DMAC_CHCTRLA_ENABLE;
#else
  //it's tough to raise an error about not having DMA in the IDE- try to fall back to regular software transfer
  writeBuffer(txdata,n);//just write the data without DMA
#endif
}

void TinyScreen::initDMA(void){
#if defined(ARDUINO_ARCH_SAMD)
  //probably on by default
  PM->AHBMASK.reg |= PM_AHBMASK_DMAC ;
  PM->APBBMASK.reg |= PM_APBBMASK_DMAC ;
  NVIC_EnableIRQ( DMAC_IRQn ) ;

  DMAC->BASEADDR.reg = (uint32_t)descriptor_section;
  DMAC->WRBADDR.reg = (uint32_t)wrb;
  DMAC->CTRL.reg = DMAC_CTRL_DMAENABLE | DMAC_CTRL_LVLEN(0xf);
#else
  //it's tough to raise an error about not having DMA in the IDE- try to fall back to regular software transfer
  //ignore init
#endif
}