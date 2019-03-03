
/*
   Segway Battery Diagnostics
   by Martin Bogomolni <martinbogo@gmail.com>

   This code is originally copyright 2019, and under the MIT License

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/

#define VERSION "1.57"

#include <Wire.h>
#include "config.h"

#ifdef I2C_LCD_DISPLAY
#include "i2c_lcd.h"
#endif

#ifdef SPI_OLED_DISPLAY
#include "spi_oled.h"
#endif

typedef struct {
  unsigned char chk;
  unsigned char msb;
  unsigned char lsb;
  unsigned char sum;
} PACKET;

typedef struct {
  unsigned int chk : 8;
  unsigned int temp : 12;
} TEMP_SENSOR;

typedef struct {
  unsigned int chk : 8;
  unsigned int voltage : 10;
} CGROUP;

typedef struct {
  unsigned char chk : 8;
  unsigned char msb : 8;
  unsigned char lsb : 8;
} DEFPACKET;

char serialnumber[13];
char revision[3];

/*
   Currently known registers

   0x17 0xD7 hold the temperature sensor info in 4 reads
   0x56 0x96 are used for battery group voltage in 23 reads
   0xC6 has the battery revision and serial number in 37 reads

   Currently unidentified registers

   0x6
   0xC 0xCC is unknown but may contain battery status information

*/

String serialnum;
long DisplayRenderMillis;
long menuMillis;
long blinkMillis;
long DisplayRenderInterval = 100;
long DisplayBlinkInterval = 5000 ;
long menuInterval = 200;

TEMP_SENSOR tempsensor[TSENSORS];
CGROUP cgroup[CELLGROUPS];
DEFPACKET dpacket[31];

void setup() {
  // By using the Wire library, the internal pullup resistors are automatically engaged
  Wire.begin();        // join i2c bus (address optional for master)
  Wire.setClock(400000);
  Serial.begin(9600);  // start serial for output
#ifdef SPI_OLED_DISPLAY
  initDisplay();
#endif
  introMessage();
  showMenu();
}

void loop() {
  
  unsigned long currentMillis = millis();

#ifdef SERIAL_DISPLAY
  if (currentMillis - menuMillis > menuInterval ) {
    menuMillis = currentMillis;
    doMenuInput();
  }
#endif

#ifdef SPI_OLED__DISPLAY
  if (currentMillis - DisplayRenderMillis > DisplayRenderInterval ) {
    DisplayRenderMillis = currentMillis;
    clearDisplay();
    updateDisplay();
  }
#endif

  if (currentMillis - blinkMillis > DisplayBlinkInterval ) {
    blinkMillis = currentMillis;
  }
}

int readPacket(int regval, PACKET &packet) {
  unsigned char sum;
  unsigned char chk;
  unsigned char msb;
  unsigned char lsb;

  memset(&packet, 0, sizeof(packet));
  Wire.beginTransmission(TYPE);
  Wire.write(regval);
  int result = Wire.endTransmission();
  if ( result ) {
#ifdef SERIAL_DISPLAY
    Serial.print("I2C communication failed with error: [");
    Serial.print(result, DEC);
    Serial.print("] ");

    switch (result) {
      case 1:
        Serial.println("XMIT buf too small");
        break;
      case 2:
        Serial.println("NACK xmit of addr");
        break;
      case 3:
        Serial.println("NACK xmit of data");
        break;
      case 4:
        Serial.println("I2C unknown err");
        break;
    }
#endif
    packet.sum = 0xff;
    packet.msb = 0x0;
    packet.lsb = 0x0;
    packet.chk = 0x0;
    return result;
  }

  Wire.requestFrom(TYPE, 3);
  while (Wire.available()) {
    packet.chk = Wire.read();
    packet.msb = Wire.read();
    packet.lsb = Wire.read();
    packet.sum = (regval + packet.chk + packet.msb + packet.lsb + 1) % 64;
  }

  return 0;
}

int readSerialNumber() {
  PACKET packet[37]; // We need to read 37 packets from register 198/0xC6
  PACKET temppacket; // To put the packet in the right place, in the right order
  char serial[16];
  char rev[4];
  int result;

  memset(serial, 0, sizeof(serial));
  memset(rev, 0, sizeof(rev));

  for ( int itor = 0; itor < 37; itor++ ) {
    result = readPacket(0xC6, temppacket);
    if ( result == -1 ) {
      return 1;
    }
    if ( temppacket.sum == 0xff && temppacket.chk == 0x0 && temppacket.msb == 0x0 && temppacket.lsb == 0x0 ) {
#ifdef SERIAL_DISPLAY
      Serial.println("I2C ERR");
#endif
      return 1;
    }
    if ( temppacket.sum != 0 ) {
#ifdef SERIAL_DISPLAY
      Serial.println("PACKET CHECKSUM INVALID");
#endif
      return 1;
    }
    memcpy(&packet[temppacket.msb], &temppacket, sizeof(temppacket));
  }

  for ( int itor; itor < 12; itor++ ) {
    serial[itor] = packet[itor + 21].lsb;
  }

  for ( int itor = 0; itor < 2; itor++ ) {
    rev[itor] = packet[itor + 12].lsb;
  }

  memset(serialnumber, 0, sizeof(serialnumber));
  memset(revision, 0, sizeof(revision));
  memcpy(serialnumber, serial, 12);
  memcpy(revision, rev, 2);

#ifdef SERIAL_DISPLAY
  Serial.print("Serial Number: ");
  Serial.print(serialnumber);
  Serial.print(" Rev:");
  Serial.println(revision);
#endif

  return 0;
}

void readTemps(void) {
  PACKET temppacket;
  int result;
  int avgtemp;
  for (int i = 0; i < TSENSORS; i++) {
    result = readPacket(0x17, temppacket); // 0x17 contains temperature data
    if ( result != 0 ) {
      return;
    }
    if ( temppacket.sum == 0xff ) {
#ifdef SERIAL_DISPLAY
      Serial.println("I2C ERR");
#endif
      return;
    }
    if ( temppacket.sum != 0 ) {
#ifdef SERIAL_DISPLAY
      Serial.println("PACKET CHECKSUM INVALID");
#endif
      return;
    }
    tempsensor[i].temp = word(temppacket.msb, temppacket.lsb) & 0xFFF; // mask off 12 bits ADC
    float ctemp = tempsensor[i].temp * 0.0625; // Sensor -256C to +256C scaled by 4096
#ifdef SERIAL_DISPLAY
    Serial.print("Temperature in deg C: ");
    Serial.println(ctemp);
#endif
  }

  memset(&temppacket, 0, sizeof(temppacket));
  result = readPacket(0x55, temppacket); // 85/0x55 149/0x95 contains pack average temp
  avgtemp = word(temppacket.msb, temppacket.lsb) & 0xFFF; // mask off 12 bits ADC
  float atemp = avgtemp * 0.0625; // Sensor -256C to +256C scaled by 4096
#ifdef SERIAL_DISPLAY
  Serial.print("Average Pack Temperature in deg C: ");
  Serial.println(atemp);
#endif
}

void readVoltages(void) {
  PACKET temppacket;
  int number;
  float packvoltage = 0;
  int result;

  for (int itor = 0; itor < CELLGROUPS; itor++) {
    result = readPacket(0x56, temppacket); // Both 0x56 and 0x96 registers hold the battery voltages, mirrored registers!
    if ( result != 0 ) {
      return;
    }
    if ( temppacket.sum == 0xff && temppacket.chk == 0x0 && temppacket.msb == 0x0 && temppacket.lsb == 0x0 ) {
#ifdef SERIAL_DISPLAY
      Serial.println("I2C ERR");
#endif
      return;
    }
    if ( temppacket.sum != 0 ) {
#ifdef SERIAL_DISPLAY
      Serial.println("PACKET CHECKSUM INVALID");
#endif
      return;
    }
    number = word(temppacket.msb, temppacket.lsb) >> 11; // top 5 bits are cell group
    cgroup[number].chk = temppacket.chk;
    cgroup[number].voltage = word(temppacket.msb, temppacket.lsb) & 0x3FF; // 10 bits of ADC
  }

  for (int itor = 0; itor < CELLGROUPS; itor++) {
    float cellvoltage = ( cgroup[itor].voltage * 7.8201 ) / 1000;
    Serial.print(" Cell group ");
    Serial.print(itor);
    Serial.print(" voltage is ");
    if ( cgroup[itor].voltage < 1023 ) {
      Serial.println(cellvoltage);
    } else {
      Serial.println("ERROR/INVALID");
    }
    if ( cgroup[itor].voltage < 1023 ) {
      packvoltage = packvoltage + cellvoltage;
    }
  }

  Serial.print("Pack voltage: ");
  Serial.print((float)packvoltage);
  Serial.println(" V");
}

void readUnknown() {
  PACKET temppacket;
  int result;

  memset(&temppacket, 0, sizeof(temppacket));
  for (int itor = 0; itor < 0x1F; itor++) {
    result = readPacket(0xC, temppacket); // 0xC and 0xCC are mirrored registers with unknown data
    if ( result != 0 ) {
      return;
    }
    if ( temppacket.sum == 0xff && temppacket.chk == 0x0 && temppacket.msb == 0x0 && temppacket.lsb == 0x0 ) {
#ifdef SERIAL_DISPLAY
      Serial.println("I2C ERR");
#endif
      return;
    }
    if ( temppacket.sum != 0 ) {
#ifdef SERIAL_DISPLAY
      Serial.println("PACKET CHECKSUM INVALID");
#endif
      return;
    }
#ifdef SERIAL_DISPLAY
    Serial.print("msb [");
    printBits(temppacket.msb);
    Serial.print(" ");
    Serial.print(temppacket.msb, DEC);
    Serial.print("] ");
    Serial.print("lsb [");
    printBits(temppacket.lsb);
    Serial.print(" ");
    Serial.print(temppacket.lsb, DEC);
    Serial.print("] ");
    Serial.print("ASCII [");
    Serial.write(temppacket.lsb);
    Serial.println("]");
#endif
  }
}

void readEveryRegister(void) {
  PACKET temppacket;
  int number;
  float packvoltage = 0;
  int result;
  char buf[3];

  for (int itor = 0; itor <= 0xFF; itor++) {
    for (int ytor = 0; ytor < 5; ytor++ ) {
      memset(&temppacket, 0, sizeof(temppacket));
      result = readPacket(itor, temppacket);
      if ( temppacket.sum == 0xff && temppacket.chk == 0x0 && temppacket.msb == 0x0 && temppacket.lsb == 0x0 ) {
#ifdef SERIAL_DISPLAY
        Serial.print("Register: ");
        Serial.print(itor, DEC);
        Serial.println("I2C ERR");
#endif
        continue;
      }
      if ( temppacket.msb == 0x0 && temppacket.lsb == 0x2 ) {
        continue; // no interesting data at this register
      }
#ifdef SERIAL_DISPLAY
      Serial.print("address [");
      Serial.print(itor, DEC);
      Serial.print(":0x");
      Serial.print(itor, HEX);
      Serial.print("] ");
      Serial.print("msb [");
      printBits(temppacket.msb);
      Serial.print(" ");
      Serial.print(temppacket.msb, DEC);
      Serial.print("] ");
      Serial.print("lsb [");
      printBits(temppacket.lsb);
      Serial.print(" ");
      Serial.print(temppacket.lsb, DEC);
      Serial.print("] ");
      Serial.print("ASCII [");
      Serial.write(temppacket.lsb);
      Serial.println("]");
#endif
    }
  }
}

void introMessage() {
#ifdef I2C_LCD_DISPLAY
  strcpy(i2cdisplay.line1, "Seg Batt Diag");
  strcpy(i2cdisplay.line2, "V ");
  strcat(i2cdisplay.line2, VERSION);
  updateDisplay();
#endif
#ifdef SPI_OLED_DISPLAY
  //strcpy(oleddisplay.line1, "Seg Batt Diag");
  //strcpy(oleddisplay.line2, "V ");
  //strcat(oleddisplay.line2, VERSION);
  updateDisplay();
#endif
#ifdef SERIAL_DISPLAY
  Serial.println("Segway Battery Diagnostics");
  Serial.println("(C) 2019 Martin Bogomolni <martinbogo@gmail.com>");
  Serial.println("MIT License");
  Serial.println();
  Serial.println("RECCOMENDED : Use a 12V power supply to your Arduino then");
  Serial.println("provide 12V from VIN to BAT ENABLE (J5) to activate the");
  Serial.println("Segway Battery BMS and read voltages.");
  Serial.println("");
  Serial.println("If battery does not respond, briefly disconnect the BAT");
  Serial.println("ENABLE pin and try again.");
  Serial.println("");
  Serial.println("Connect SCL (j6) and SDA (J7) to Arduino I2C pins.");
  Serial.println("");
#endif
}

void blinkLed() {
  pinMode(13, LED_BUILTIN);
  for ( int i = 0; i < 3; i++  ) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(50);
    digitalWrite(LED_BUILTIN, LOW);
    delay(50);
  }
}

void showMenu(void) {
#ifdef SERIAL_DISPLAY
  Serial.println("V) Read raw cell group voltages");
  Serial.println("T) Read temperature sensors");
  Serial.println("S) Read serial number");
  Serial.println("U) Read data from register 0xC/0xCC");
  Serial.println("R) Read all registers once");
  Serial.println("H) Help! Show this menu");
  Serial.println("");
  Serial.println("Press key to select menu item:");
#endif
}

#ifdef SERIAL_DISPLAY
void doMenuInput(void) {
  if (Serial.available() > 0) {
    int inByte = Serial.read();
    Serial.write(inByte);
    Serial.println(".");
    switch (inByte) {
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

void printBits(byte myByte) {
  for (byte mask = 0x80; mask; mask >>= 1) {
    if (mask  & myByte)
      Serial.print('1');
    else
      Serial.print('0');
  }
}
#endif
