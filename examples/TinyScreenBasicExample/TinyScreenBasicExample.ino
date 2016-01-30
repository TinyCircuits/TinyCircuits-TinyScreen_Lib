//-------------------------------------------------------------------------------
//  TinyCircuits TinyScreen/TinyScreen+ Basic Example
//  Last Updated 26 January 2016
//  
//  This example shows the basic functionality of the TinyScreen library,
//  including drawing, writing bitmaps, and printing text
//
//  Written by Ben Rose for TinyCircuits, https://tiny-circuits.com
//
//-------------------------------------------------------------------------------

#include <Wire.h>
#include <SPI.h>
#include <TinyScreen.h>

//Library must be passed the board type
//TinyScreenDefault for TinyScreen shields
//TinyScreenAlternate for alternate address TinyScreen shields
//TinyScreenPlus for TinyScreen+
TinyScreen display = TinyScreen(TinyScreenPlus);

//This is an example 17x12 pixel bitmap using TS library color definitions
unsigned char flappyBirdBitmap[204]={
  TS_8b_Blue,TS_8b_Blue,TS_8b_Blue,TS_8b_Blue,TS_8b_Blue,TS_8b_Blue,TS_8b_Black,TS_8b_Black,TS_8b_Black,TS_8b_Black,TS_8b_Black,TS_8b_Black,TS_8b_Blue,TS_8b_Blue,TS_8b_Blue,TS_8b_Blue,TS_8b_Blue,
  TS_8b_Blue,TS_8b_Blue,TS_8b_Blue,TS_8b_Blue,TS_8b_Black,TS_8b_Black,TS_8b_White,TS_8b_White,TS_8b_White,TS_8b_Black,TS_8b_White,TS_8b_White,TS_8b_Black,TS_8b_Blue,TS_8b_Blue,TS_8b_Blue,TS_8b_Blue,
  TS_8b_Blue,TS_8b_Blue,TS_8b_Blue,TS_8b_Black,TS_8b_White,TS_8b_White,TS_8b_Yellow,TS_8b_Yellow,TS_8b_Black,TS_8b_White,TS_8b_White,TS_8b_White,TS_8b_White,TS_8b_Black,TS_8b_Blue,TS_8b_Blue,TS_8b_Blue,
  TS_8b_Blue,TS_8b_Black,TS_8b_Black,TS_8b_Black,TS_8b_Black,TS_8b_Yellow,TS_8b_Yellow,TS_8b_Yellow,TS_8b_Black,TS_8b_White,TS_8b_White,TS_8b_White,TS_8b_Black,TS_8b_White,TS_8b_Black,TS_8b_Blue,TS_8b_Blue,
  TS_8b_Black,TS_8b_White,TS_8b_White,TS_8b_White,TS_8b_White,TS_8b_Black,TS_8b_Yellow,TS_8b_Yellow,TS_8b_Black,TS_8b_White,TS_8b_White,TS_8b_White,TS_8b_Black,TS_8b_White,TS_8b_Black,TS_8b_Blue,TS_8b_Blue,
  TS_8b_Black,TS_8b_White,TS_8b_White,TS_8b_White,TS_8b_White,TS_8b_White,TS_8b_Black,TS_8b_Yellow,TS_8b_Yellow,TS_8b_Black,TS_8b_White,TS_8b_White,TS_8b_White,TS_8b_White,TS_8b_Black,TS_8b_Blue,TS_8b_Blue,
  TS_8b_Black,TS_8b_Yellow,TS_8b_White,TS_8b_White,TS_8b_White,TS_8b_Yellow,TS_8b_Black,TS_8b_Yellow,TS_8b_Yellow,TS_8b_Yellow,TS_8b_Black,TS_8b_Black,TS_8b_Black,TS_8b_Black,TS_8b_Black,TS_8b_Black,TS_8b_Blue,
  TS_8b_Blue,TS_8b_Black,TS_8b_Yellow,TS_8b_Yellow,TS_8b_Yellow,TS_8b_Black,TS_8b_Yellow,TS_8b_Yellow,TS_8b_Yellow,TS_8b_Black,TS_8b_Red,TS_8b_Red,TS_8b_Red,TS_8b_Red,TS_8b_Red,TS_8b_Red,TS_8b_Black,
  TS_8b_Blue,TS_8b_Blue,TS_8b_Black,TS_8b_Black,TS_8b_Black,TS_8b_Yellow,TS_8b_Yellow,TS_8b_Yellow,TS_8b_Yellow,TS_8b_Black,TS_8b_Red,TS_8b_Black,TS_8b_Black,TS_8b_Black,TS_8b_Black,TS_8b_Black,TS_8b_Black,
  TS_8b_Blue,TS_8b_Blue,TS_8b_Black,TS_8b_Yellow,TS_8b_Yellow,TS_8b_Yellow,TS_8b_Yellow,TS_8b_Yellow,TS_8b_Yellow,TS_8b_Black,TS_8b_Red,TS_8b_Red,TS_8b_Red,TS_8b_Red,TS_8b_Red,TS_8b_Black,TS_8b_Blue,
  TS_8b_Blue,TS_8b_Blue,TS_8b_Blue,TS_8b_Black,TS_8b_Black,TS_8b_Yellow,TS_8b_Yellow,TS_8b_Yellow,TS_8b_Yellow,TS_8b_Yellow,TS_8b_Black,TS_8b_Black,TS_8b_Black,TS_8b_Black,TS_8b_Black,TS_8b_Blue,TS_8b_Blue,
  TS_8b_Blue,TS_8b_Blue,TS_8b_Blue,TS_8b_Blue,TS_8b_Blue,TS_8b_Black,TS_8b_Black,TS_8b_Black,TS_8b_Black,TS_8b_Black,TS_8b_Blue,TS_8b_Blue,TS_8b_Blue,TS_8b_Blue,TS_8b_Blue,TS_8b_Blue,TS_8b_Blue
};

void setup(void) {
  Wire.begin();//initialize I2C before we can initialize TinyScreen- not needed for TinyScreen+
  display.begin();
  //setBrightness(brightness);//sets main current level, valid levels are 0-15
  display.setBrightness(10);
}

void loop() {
  hardwareDrawCommands();
  drawPixels();
  drawBitmap();
  //setFlip(boolean);//done in hardware on the SSD1331
  display.setFlip(true);
  delay(1000);
  display.setFlip(false);
  delay(1000);
  writeText();
  delay(1000);
  readInput();
}

void hardwareDrawCommands(){
  //Accelerated drawing commands are executed by the display controller
  //clearScreen();//clears entire display- the same as clearWindow(0,0,96,64)
  display.clearScreen();
  //drawRect(x stary, y start, width, height, fill, 8bitcolor);//sets specified OLED controller memory to an 8 bit color, fill is a boolean- TSRectangleFilled or TSRectangleNoFill
  display.drawRect(10,10,76,44,TSRectangleFilled,TS_8b_Red);
  //drawRect(x stary, y start, width, height, fill, red, green, blue);//like above, but uses 6 bit color values. Red and blue ignore the LSB.
  display.drawRect(15,15,66,34,TSRectangleFilled,20,30,60);
  //clearWindow(x start, y start, width, height);//clears specified OLED controller memory
  display.clearWindow(20,20,56,24);
  //drawLine(x1, y1, x2, y2, 8bitcolor);//draw a line from (x1,y1) to (x2,y2) with an 8 bit color
  display.drawLine(0,0,95,63,TS_8b_Green);
  //drawLine(x1, y1, x2, y2, red, green, blue);//like above, but uses 6 bit color values. Red and blue ignore the LSB.
  display.drawLine(0,63,95,0,0,63,0);
  delay(1000);
  //use 16 bit version of drawLine to fade a rectangle from blue to green:
  for(int i=0;i<64;i++){
    display.drawLine(0,i,95,i,0,i,63-i);
  }
  delay(1000);
}

void drawPixels(){
  //writing pixels one by one is slow, but neccessary for drawing shapes other than lines and rectangles
  //we'll implement a simple circle drawing algorithm from http://en.wikipedia.org/wiki/Midpoint_circle_algorithm
  drawCircle(40,30,30,TS_8b_Red);
  drawCircle(45,30,25,TS_8b_Yellow);
  drawCircle(50,30,20,TS_8b_Blue);
  drawCircle(55,30,15,TS_8b_Brown);
  drawCircle(60,30,10,TS_8b_Green);
  drawCircle(65,30,5,TS_8b_Black);
  delay(1000);
}

void drawCircle(int x0, int y0, int radius, uint8_t color)
{
  int x = radius;
  int y = 0;
  int radiusError = 1-x;
 
  while(x >= y)
  {
    //drawPixel(x,y,color);//set pixel (x,y) to specified color. This is slow because we need to send commands setting the x and y, then send the pixel data.
    display.drawPixel(x + x0, y + y0, color);
    display.drawPixel(y + x0, x + y0, color);
    display.drawPixel(-x + x0, y + y0, color);
    display.drawPixel(-y + x0, x + y0, color);
    display.drawPixel(-x + x0, -y + y0, color);
    display.drawPixel(-y + x0, -x + y0, color);
    display.drawPixel(x + x0, -y + y0, color);
    display.drawPixel(y + x0, -x + y0, color);
    y++;
    if (radiusError<0)
    {
      radiusError += 2 * y + 1;
    }
    else
    {
      x--;
      radiusError += 2 * (y - x) + 1;
    }
  }
}

void drawBitmap(){
  //set a background that matches
  display.drawRect(0,0,96,64,TSRectangleFilled,TS_8b_Blue);
  //let's set up for a bitmap at (40,30) that is 17 pixels wide and 12 pixels tall:
  //setX(x start, x end);//set OLED RAM to x start, wrap around at x end
  display.setX(40,40+17-1);
  //setY(y start, y end);//set OLED RAM to y start, wrap around at y end
  display.setY(30,30+12-1);
  //now start a data transfer
  display.startData();
  //writeBuffer(buffer,count);//optimized write of a large buffer of 8 bit data.
  display.writeBuffer(flappyBirdBitmap,17*12);
  display.endTransfer();
  delay(1000);
  
}

void writeText(){
  display.clearScreen();
  //setFont sets a font info header from font.h
  //information for generating new fonts is included in font.h
  display.setFont(thinPixel7_10ptFontInfo);
  //getPrintWidth(character array);//get the pixel print width of a string
  int width=display.getPrintWidth("Example Text!");
  //setCursor(x,y);//set text cursor position to (x,y)- in this example, the example string is centered
  display.setCursor(48-(width/2),10);
  //fontColor(text color, background color);//sets text and background color
  display.fontColor(TS_8b_Green,TS_8b_Black);
  display.print("Example Text!");
  display.setCursor(15,25);
  display.fontColor(TS_8b_Blue,TS_8b_Black);
  display.print("More example Text!");
  display.setCursor(3,40);
  display.fontColor(TS_8b_Red,TS_8b_Black);
  display.print("(Does not wrap)");
  delay(1000);
}

void readInput() {
  display.fontColor(TS_8b_White,TS_8b_Black);
  display.clearScreen();
  display.setCursor(48 - (display.getPrintWidth("Press a Button!") / 2), 32 - (display.getFontHeight() / 2));
  display.print("Press a Button!");
  unsigned long startTime = millis();
  while (millis() - startTime < 3000)buttonLoop();
  display.clearScreen();
  display.setFlip(true);
  display.setCursor(48 - (display.getPrintWidth("Press a Button!") / 2), 32 - (display.getFontHeight() / 2));
  display.print("Press a Button!");
  startTime = millis();
  while (millis() - startTime < 3000)buttonLoop();
  display.clearScreen();
  display.setFlip(false);
}

void buttonLoop() {
  display.setCursor(0, 0);
  //getButtons() function can be used to test if any button is pressed, or used like:
  //getButtons(TSButtonUpperLeft) to test a particular button, or even like:
  //getButtons(TSButtonUpperLeft|TSButtonUpperRight) to test multiple buttons
  //results are flipped as you would expect when setFlip(true)
  if (display.getButtons(TSButtonUpperLeft)) {
    display.println("Pressed!");
  } else {
    display.println("          ");
  }
  display.setCursor(0, 54);
  if (display.getButtons(TSButtonLowerLeft)) {
    display.println("Pressed!");
  } else {
    display.println("          ");
  }
  display.setCursor(95 - display.getPrintWidth("Pressed!"), 0);
  if (display.getButtons(TSButtonUpperRight)) {
    display.println("Pressed!");
  } else {
    display.println("          ");
  }
  display.setCursor(95 - display.getPrintWidth("Pressed!"), 54);
  if (display.getButtons(TSButtonLowerRight)) {
    display.println("Pressed!");
  } else {
    display.println("          ");
  }
}