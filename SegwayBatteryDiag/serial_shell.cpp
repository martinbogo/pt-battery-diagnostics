
#include "config.h"

#ifdef SERIAL_SHELL
#include "serial_shell.h"
 
void doMenuInput(void) {
  if (Serial.available() > 0) {
    uint8_t input = Serial.read();
    switch (input) {
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

const char menuText[] PROGMEM = "V) Read voltages\nC) Read SoC\nT) Read Temps\nS) Read Serial no.\nU) Read 0xC/0xCC\nR) Read all\nH) Help\n\nPress Key\n\n";
  
void showMenu(void) {
  for (byte k = 0; k < strlen_P(menuText); k++) {
    char myChar = pgm_read_byte_near(menuText + k);
    Serial.print(myChar);
  }
}

#endif
