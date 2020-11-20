
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

#define VERSION "1.129"

#define HEARTBEAT_LED 13

#include <Wire.h>
#include "config.h"

#ifdef I2C_LCD_DISPLAY
#include "i2c_lcd.h"
#endif

#ifdef SPI_OLED_DISPLAY
#include "spi_oled.h"
#endif

#ifdef SERIAL_SHELL
#include "serial_shell.h"
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
   0x1D has the battery State of Charge as one byte LSB
   0xD4 has the battery amperage drain/charge as signed 16 bits ( divide by 0.128 )

   Currently unidentified registers

   0x6
   0xC 0xCC is unknown but may contain battery status information

*/

String serialnum;
long DisplayRenderMillis;
long menuMillis;
long blinkMillis;
long ledFlashMillis;
long ledFlashInterval = 5000;
long DisplayRenderInterval = 1000;
long DisplayBlinkInterval = 5000;
long menuInterval = 200;

TEMP_SENSOR tempsensor[TSENSORS];
CGROUP cgroup[CELLGROUPS];
DEFPACKET dpacket[31];

void setup() {
  // By using the Wire library, the internal pullup resistors are automatically engaged
  Wire.begin(); // join i2c bus (address optional for master)
  Wire.setClock(400000); // If 400000 doesn't work, try 100000 
#ifdef HEARTBEAT_LED
  pinMode(HEARTBEAT_LED,OUTPUT);
#endif

#ifdef SPI_OLED_DISPLAY
  initDisplay();
#endif

#ifdef I2C_LCD_DISPLAY
  initDisplay();
#endif

#ifdef SERIAL_SHELL
  Serial.begin(57600);  // start serial for output
  while (!Serial);
  delay(100);
  introMessage();
  delay(100);
  showMenu();
#endif

}

void loop() {
  // timing loop
  unsigned long currentMillis = millis();

#ifdef HEARTBEAT_LED
  if ( currentMillis - ledFlashMillis > ledFlashInterval ) {
    ledFlashMillis = currentMillis;
    digitalWrite(HEARTBEAT_LED, HIGH);
    delay(10);
    digitalWrite(HEARTBEAT_LED, LOW);
    delay(50);
    digitalWrite(HEARTBEAT_LED, HIGH);
    delay(10);
    digitalWrite(HEARTBEAT_LED, LOW);
  }
#endif 

#ifdef SERIAL_SHELL
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

#ifdef I2C_LCD_DISPLAY
  if (currentMillis - DisplayRenderMillis > DisplayRenderInterval ) {
    DisplayRenderMillis = currentMillis;
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
#ifdef SERIAL_SHELL
    Serial.print("I2C ERR: [");
    Serial.print(result, DEC);
    Serial.print("] ");

    switch (result) {
      case 1:
        Serial.println(F("LONG")); // data too long to fit in xmit buffer
        return -1;
        break;
      case 2:
        Serial.println(F("ADDR")); // NACK on transmit of address
        return -1;
        break;
      case 3:
        Serial.println(F("DATA")); // NACK on transmit of data
        return -1;
        break;
      case 4:
        Serial.println(F("OTHER")); // Other unknown error
        return -1;
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
      return 1;
    }
    if ( temppacket.sum != 0 ) {
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

#ifdef SERIAL_SHELL
  Serial.print(F("Serial Number: "));
  Serial.print(serialnumber);
  Serial.print(F(" Rev:"));
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
#ifdef SERIAL_SHELL
      Serial.println(F("I2C ERR"));
#endif
      return;
    }
    if ( temppacket.sum != 0 ) {
#ifdef SERIAL_SHELL
      Serial.println(F("CRC ERR"));
#endif
      return;
    }
    tempsensor[i].temp = word(temppacket.msb, temppacket.lsb) & 0xFFF; // mask off 12 bits ADC
    float ctemp = tempsensor[i].temp * 0.0625; // Sensor -256C to +256C scaled by 4096
#ifdef SERIAL_SHELL
    Serial.print(ctemp);
    Serial.println(F(" °C"));
#endif
  }

  memset(&temppacket, 0, sizeof(temppacket));
  result = readPacket(0x55, temppacket); // 85/0x55 149/0x95 contains pack average temp
  avgtemp = word(temppacket.msb, temppacket.lsb) & 0xFFF; // mask off 12 bits ADC
  float atemp = avgtemp * 0.0625; // Sensor -256C to +256C scaled by 4096
#ifdef SERIAL_SHELL
  Serial.print(atemp);
  Serial.println(F(" °C Pack"));
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
#ifdef SERIAL_SHELL
      Serial.println(F("I2C ERR"));
#endif
      return;
    }
    if ( temppacket.sum != 0 ) {
#ifdef SERIAL_SHELL
      Serial.println(F("CRC ERR"));
#endif
      return;
    }
    number = word(temppacket.msb, temppacket.lsb) >> 11; // top 5 bits are cell group
    cgroup[number].chk = temppacket.chk;
    cgroup[number].voltage = word(temppacket.msb, temppacket.lsb) & 0x3FF; // 10 bits of ADC
  }

  for (int itor = 0; itor < CELLGROUPS; itor++) {
    float cellvoltage = ( cgroup[itor].voltage * 7.8201 ) / 1000;
#ifdef SERIAL_SHELL
    Serial.print(F(" Cell group "));
    Serial.print(itor);
    Serial.print(F(" voltage is "));
    if ( cgroup[itor].voltage < 1023 ) {
      Serial.println(cellvoltage);
    } else {
      Serial.println(F("ERR"));
    }
#endif
    if ( cgroup[itor].voltage < 1023 ) {
      packvoltage = packvoltage + cellvoltage;
    }
  }

  Serial.print(F("Pack voltage: "));
  Serial.print((float)packvoltage);
  Serial.println(F(" V"));
}

float readStateOfCharge() {
  float average_soc = 100;
  PACKET temppacket;
  int result;
  int number;
  float stateofcharge;

  memset(&temppacket, 0, sizeof(temppacket));

  result = readPacket(0x1D, temppacket);
  if ( result != 0 ) {
    return 1;
  }
  if ( temppacket.sum == 0xff && temppacket.chk == 0x0 && temppacket.msb == 0x0 && temppacket.lsb == 0x0 ) {
#ifdef SERIAL_SHELL
    Serial.println(F("I2C ERR"));
#endif
    return 1;
  }
  if ( temppacket.sum != 0 ) {
#ifdef SERIAL_SHELL
    Serial.println(F("CRC ERR"));
#endif
    return 1;
  }

  number = word(temppacket.msb, temppacket.lsb) & 0xFF; // 8 bits of SOC
  stateofcharge = average_soc / number;
#ifdef SERIAL_SHELL
  Serial.print(stateofcharge, 2);
  Serial.println(F("% SoC"));
#endif
  return stateofcharge;
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
#ifdef SERIAL_SHELL
      Serial.println("I2C ERR");
#endif
      return;
    }
    if ( temppacket.sum != 0 ) {
#ifdef SERIAL_SHELL
      Serial.println(F("CRC ERR"));
#endif
      return;
    }
#ifdef SERIAL_SHELL
    Serial.print(F("msb ["));
    printBits(temppacket.msb);
    Serial.print(F(" "));
    Serial.print(temppacket.msb, DEC);
    Serial.print(F("] "));
    Serial.print(F("lsb ["));
    printBits(temppacket.lsb);
    Serial.print(F(" "));
    Serial.print(temppacket.lsb, DEC);
    Serial.print(F("] "));
    Serial.print(F("ASCII ["));
    Serial.write(temppacket.lsb);
    Serial.println(F("]"));
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
      if ( result == -1 ) { return; }
      if ( temppacket.sum == 0xff && temppacket.chk == 0x0 && temppacket.msb == 0x0 && temppacket.lsb == 0x0 ) {
#ifdef SERIAL_SHELL
        Serial.print(F("REG: "));
        Serial.print(itor, DEC);
        Serial.println(F("I2C ERR"));
#endif
        continue;
      }
      if ( temppacket.msb == 0x0 && temppacket.lsb == 0x2 ) {
        continue; // no interesting data at this register
      }
#ifdef SERIAL_SHELL
      Serial.print(F("address ["));
      Serial.print(itor, DEC);
      Serial.print(F(":0x"));
      Serial.print(itor, HEX);
      Serial.print(F("] "));
      Serial.print(F("msb ["));
      printBits(temppacket.msb);
      Serial.print(F(" "));
      Serial.print(temppacket.msb, DEC);
      Serial.print(F("] "));
      Serial.print(F("lsb ["));
      printBits(temppacket.lsb);
      Serial.print(F(" "));
      Serial.print(temppacket.lsb, DEC);
      Serial.print(F("] "));
      Serial.print(F("ASCII ["));
      Serial.write(temppacket.lsb);
      Serial.println(F("]"));
#endif
    }
  }
}

void introMessage() {

#ifdef I2C_LCD_DISPLAY
  storeLine(1, "Segway Battery Diagnostics");
  storeLine(2, "V ");
  storeLine(3, VERSION);
  updateDisplay();
#endif

#ifdef SPI_OLED_DISPLAY
  updateDisplay();
#endif

#ifdef SERIAL_SHELL
  Serial.println(F("Segway Battery Diagnostics"));
  Serial.println(F("Martin Bogomolni ©2020"));
  Serial.print(F("V: "));
  Serial.print(VERSION);
  Serial.println(F("\n"));
#endif

}
