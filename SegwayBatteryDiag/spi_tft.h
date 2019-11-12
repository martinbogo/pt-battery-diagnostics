
#pragma once
#ifndef SPI_TFT_H
#define SPI_TFT_H

#include <Adafruit_GFX.h>
#include <TFT.h>
#include <SPI.h>

#define CS        10
#define RST        9 
#define DC         8

// Color definitions
#define BLACK           0x0000
#define BLUE            0x001F
#define RED             0xF800
#define GREEN           0x07E0
#define CYAN            0x07FF
#define MAGENTA         0xF81F
#define YELLOW          0xFFE0
#define WHITE           0xFFFF

extern void initDisplay();
extern void updateDisplay();
extern void lcdTestPattern();
extern void clearDisplay();
extern void writeText(int, String);
extern void drawBatteryLine(int, int);

#endif
