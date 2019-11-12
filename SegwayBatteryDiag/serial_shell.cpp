
#include "config.h"

#ifdef SERIAL_SHELL
#include "serial_shell.h"
 
void doMenuInput(void) {
  if (Serial.available() > 0) {
    int inuint8_t = Serial.read();
    Serial.write(inuint8_t);
    Serial.println(".");
    switch (inuint8_t) {
      case 'C': readStateOfCharge(); break;
      case 'c': readStateOfCharge(); break;
      case 'V': readVoltages(); break;
      case 'v': readVoltages(); break;
      case 'T': readTemps(); break;
      case 't': readTemps(); break;
      case 'S': readSerialNumber(); break;
      case 's': readSerialNumber(); break;
      case 'U': readUnknown(); break;
      case 'u': readUnknown(); break;
      case 'R': readEveryRegister(); break;
      case 'r': readEveryRegister(); break;
      case 'H': showMenu(); break;
      case 'h': showMenu(); break;
      default: return;
    }
  }
}

void printBits(uint8_t myuint8_t) {
  for (uint8_t mask = 0x80; mask; mask >>= 1) {
    if (mask  & myuint8_t)
      Serial.print('1');
    else
      Serial.print('0');
  }
}

void showMenu(void) {
  Serial.println("V) Read raw cell group voltages");
  Serial.println("C) Read state of charge");
  Serial.println("T) Read temperature sensors");
  Serial.println("S) Read serial number");
  Serial.println("U) Read data from register 0xC/0xCC");
  Serial.println("R) Read all registers once");
  Serial.println("H) Help! Show this menu");
  Serial.println("");
  Serial.println("Press key to select menu item:");
}

#endif
