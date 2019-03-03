
#ifndef I2C_LCD_H
#define I2C_LCD_H

#define DISPADDR 0x27
#define DISPCOLS 20
#define DISPROWS 4

struct displaymsg {
  char *line1;
  char *line2;
  char *line3;
  char *line4;
};

struct displaymsg i2cdisplay;
int spinstate;

LiquidCrystal_I2C lcd(DISPADDR, DISPCOLS, DISPROWS);

void doBlink();
void updateDisplay();

#endif
