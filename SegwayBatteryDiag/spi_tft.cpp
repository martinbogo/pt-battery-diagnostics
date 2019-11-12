
#include "config.h"

#ifdef SPI_TFT_DISPLAY
#include "spi_tft.h"

float pi = 3.1415926;

int refreshdisplay;

String line1;
String line2;
String line3;
String line4;
String line5;
String line6;
String line7;

TFT display = TFT(CS, DC, RST);

void initDisplay() {
  display.begin();
  display.fillScreen(BLACK);
  lcdTestPattern();
  delay(500);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.fillScreen(BLACK);
}

void clearDisplay() {
  display.fillScreen(BLACK);
}

void writeText(int line, String text) {
  switch (line) {
    case 1:
      line1 = text;
      break;
    case 2:
      line2 = text;
      break;
    case 3:
      line3 = text;
      break;
    case 4:
      line4 = text;
      break;
    case 5:
      line5 = text;
      break;
    case 6:
      line6 = text;
      break;
    case 7:
      line7 = text;
      break;
    default:
      break;
  }
  refreshdisplay=1;
}

void updateDisplay() {
  if (refreshdisplay) {
  //clearDisplay();
  display.setCursor(0,0);
  display.print(String(line1));
  display.setCursor(0,9);
  display.print(String(line2));
  display.setCursor(0,18);
  display.print(String(line3));
  display.setCursor(0,27);
  display.print(String(line4));
  display.setCursor(0,36);
  display.print(String(line5));
  display.setCursor(0,45);
  display.print(String(line6));
  display.setCursor(0,54);
  display.print(String(line7));
  refreshdisplay=0;
  }
}

void drawBatteryLine(int battery, int voltage) {
  int x=0;
  int y=10;
  if ( battery > 11 ) {
    x = 48;
    display.fillRect(x,y+((4*battery)-48),46,3,RED);
    if ( voltage > 460 || voltage < 320 ) {
      display.fillRect(x,y+((4*battery)-48),46,3,MAGENTA);
    } else {
      display.fillRect(x,y+((4*battery)-48),(0.328*(voltage-320)),3,GREEN);
    }
  } else {
    display.fillRect(x,y+((4*battery)),46,3,RED);
    if ( voltage > 460 || voltage < 320 ) {
      display.fillRect(x,y+(4*battery),46,3,MAGENTA);
    } else {
      display.fillRect(x,y+(4*battery),(0.328*(voltage-320)),3,GREEN);
    }
  }
}

void lcdTestPattern()
{
  uint8_t w,h;
  display.setAddrWindow(0, 0, 128, 128);
  
  for(h=0; h < 128; h++)
  {
    for(w=0; w<128; w++)
    {
      if(w>111){display.writePixel(w,h,WHITE);}
      else if(w>95){display.writePixel(w,h,BLUE);}
      else if(w>79){display.writePixel(w,h,GREEN);}
      else if(w>63){display.writePixel(w,h,CYAN);}
      else if(w>47){display.writePixel(w,h,RED);}
      else if(w>31){display.writePixel(w,h,MAGENTA);}
      else if(w>15){display.writePixel(w,h,YELLOW);}
      else {display.writePixel(w,h,BLACK);}
    }
  }
  display.endWrite();
}

#endif
