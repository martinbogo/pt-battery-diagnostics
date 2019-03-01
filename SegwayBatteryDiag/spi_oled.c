
#ifdef __cplusplus
extern "C" {
#endif

#ifdef SPI_OLED_DISPLAY

Adafruit_SSD1331 display = Adafruit_SSD1331(cs, dc, rst);

int refreshdisplay;
struct displaymsg oleddisplay;

void initDisplay() {
  display.begin();
}

void updateDisplay() {
  if (refreshdisplay) {
    display.fillScreen(BLACK);
    refreshdisplay = 0;
  }
  display.setCursor(0, 0);
  display.print(oleddisplay.line1);
  display.setCursor(0, 9);
  display.print(oleddisplay.line2);
  display.setCursor(0, 18);
  display.print(oleddisplay.line3);
  display.setCursor(0, 27);
  display.print(oleddisplay.line4);
}

void lcdTestPattern()
{
  uint8_t w,h;
  display.setAddrWindow(0, 0, 96, 64);
  
  for(h=0; h<64; h++)
  {
    for(w=0; w<96; w++)
    {
      if(w>83){display.writePixel(WHITE);}
      else if(w>71){display.writePixel(BLUE);}
      else if(w>59){display.writePixel(GREEN);}
      else if(w>47){display.writePixel(CYAN);}
      else if(w>35){display.writePixel(RED);}
      else if(w>23){display.writePixel(MAGENTA);}
      else if(w>11){display.writePixel(YELLOW);}
      else {display.writePixel(BLACK);}
    }
  }
  display.endWrite();
}
#endif

#ifdef __cplusplus
} // extern "C"
#endif
