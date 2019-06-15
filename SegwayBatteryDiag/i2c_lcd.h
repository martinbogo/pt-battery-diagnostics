
#pragma once
#ifndef I2C_LCD_H
#define I2C_LCD_H

#include <LiquidCrystal_I2C.h>

#define DISPADDR 0x27
#define DISPCOLS 20
#define DISPROWS 4

struct displaymsg {
  char *line1;
  char *line2;
  char *line3;
  char *line4;
};

extern struct displaymsg i2cdisplay;

extern int spinstate;
extern int refreshDisplay;

extern void doBlink();
extern void updateDisplay();
extern void initDisplay();
extern void clearDisplay();

#endif
