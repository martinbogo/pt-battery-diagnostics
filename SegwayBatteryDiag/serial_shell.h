
#pragma once
#ifndef SERIAL_SHELL_H
#define SERIAL_SHELL_H

#include "Arduino.h"

void doMenuInput();
void printBits(uint8_t);
void showMenu();
void introMessage();
void readVoltages();
void readTemps();
void readUnknown();
int readSerialNumber();
void readEveryRegister();
float readStateOfCharge();

#endif
