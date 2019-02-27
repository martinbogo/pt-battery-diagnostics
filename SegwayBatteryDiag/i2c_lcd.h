#ifdef I2C_LCD_DISPLAY

#define DISPADDR 0x27
#define DISPCOLS 20
#define DISPROWS 4

struct displaymsg {
  char *line1;
  char *line2;
  char *line3;
  char *line4;
};

#endif
