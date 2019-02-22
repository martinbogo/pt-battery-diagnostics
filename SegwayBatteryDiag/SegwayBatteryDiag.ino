/*
   Segway Battery Diagnostics
   by Martin Bogomolni <martinbogo@gmail.com>


   Created 15 Jan, 2019
   Updated 20 Feb, 2019

   v 1.55

   This code is copyright 2019, and under the MIT License

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

#include <Wire.h>

//#define I2C_DISPLAY
/* If you want to use an LCD I2C display, make sure you have the
   LiquidCrystal_I2C library installed and enabled in your Arduino
   installation
*/
#ifdef I2C_DISPLAY
#include <LiquidCrystal_I2C.h>
#define DISPADDR 0x27
#define DISPCOLS 20
#define DISPROWS 4
LiquidCrystal_I2C lcd(DISPADDR, DISPCOLS, DISPROWS);
#endif

typedef struct {
  unsigned char chk;
  unsigned char msb;
  unsigned char lsb;
  unsigned char mod;
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
  unsigned char chk;
  unsigned char msb;
  unsigned char lsb;
} UNK;


// For Lithium Segway Batteries uncomment these lines
#define TYPE 0x31
#define CELLGROUPS 23
#define TSENSORS 4
// For NiMH Segway Batteries uncomment these lines
//#define TYPE 0x62
//#define CELLGROUPS 6
//#define TSENSORS 4

/*
   Currently known registers

   0x17 0xD7 hold the temperature sensor info in 4 reads
   0x56 0x96 are used for battery group voltage in 23 reads
   0xC6 has the battery revision and serial number in 37 reads

   Currently unidentified registers

   0xC 0xCC is unknown but may contain battery status information

*/

String serialnum;

TEMP_SENSOR tempsensor[TSENSORS];
CGROUP cgroup[CELLGROUPS];
UNK unknown[31];

void setup() {
  Wire.begin();        // join i2c bus (address optional for master)
  Serial.begin(9600);  // start serial for output
}

void loop() {
  introMessage();
  while (1) {
    doMenu();
  }
}

int readPacket(int regval, PACKET &packet) {
  unsigned char mod;
  unsigned char chk;
  unsigned char msb;
  unsigned char lsb;

  Wire.beginTransmission(TYPE);
  Wire.write(regval);
  int result = Wire.endTransmission();
  if ( result != 0 ) {
    Serial.print("I2C communication failed with error: [");
    Serial.print(result, DEC);
    Serial.println("]");
    packet.mod = 0xff;
    packet.msb = 0xff;
    packet.lsb = 0xff;
    packet.chk = 0xff;
    return -1;
  }

  Wire.requestFrom(TYPE, 3);
  while (Wire.available()) {
    packet.chk = Wire.read();
    packet.msb = Wire.read();
    packet.lsb = Wire.read();
    packet.mod = (packet.chk + packet.msb + packet.lsb) % 8;
  }
}

void readSerialNumber(void) {
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
      return;
    }
    if ( temppacket.chk == 0xff ) {
      Serial.println("COMMUNICATION ERROR DETECTED - CHECKSUM INVALID");
      return;
    }
    memcpy(&packet[temppacket.msb], &temppacket, sizeof(temppacket));
  }

  for ( int itor; itor < 12; itor++ ) {
    serial[itor] = packet[itor + 21].lsb;
  }

  for ( int itor = 0; itor < 2; itor++ ) {
    rev[itor] = packet[itor + 12].lsb;
  }

  Serial.print(serial);
  Serial.print(" Rev: ");
  Serial.println(rev);

}

void readTemps(void) {
  for (int i = 0; i < TSENSORS; i++) {
    Wire.beginTransmission(TYPE);
    Wire.write(0x17); // 0x17 and 0xD7 hold the temp sensor values, mirrored registers!
    int result = Wire.endTransmission();
    if ( result != 0 ) {
      Serial.print("I2C communication failed with error: [");
      Serial.print(result, DEC);
      Serial.println("]");
      return;
    }

    Wire.requestFrom(TYPE, 3);
    while (Wire.available()) {
      unsigned char t = Wire.read(); // checksum
      delay(5);
      char h = Wire.read(); // high byte
      delay(5);
      char l = Wire.read(); // low byte
      tempsensor[i].chk = t;
      tempsensor[i].temp = word(h, l) & 0xFFF; // mask off 12 bits ADC
      float ctemp = tempsensor[i].temp * 0.0625; // Sensor -256C to +256C scaled by 4096
      Serial.print("Temperature in deg C: ");
      Serial.println(ctemp);
    }
  }
}

void readVoltages(void) {
  PACKET temppacket;
  int number;
  float packvoltage = 0;
  int result;

  for (int itor = 0; itor < CELLGROUPS; itor++) {
    result = readPacket(0x56, temppacket); // Both 0x56 and 0x96 registers hold the battery voltages, mirrored registers!
    if ( result == -1 ) {
      return;
    }
    if ( temppacket.mod == 0xff ) {
      Serial.println("COMMUNICATION ERROR DETECTED - CHECKSUM INVALID");
      return;
    }
    number = word(temppacket.msb, temppacket.lsb) >> 11; // top 5 bits are cell group
    cgroup[number].chk = temppacket.chk;
    cgroup[number].voltage = word(temppacket.msb, temppacket.lsb) & 0x3FF; // 10 bits of ADC
  }

  for (int itor = 0; itor < CELLGROUPS; itor++) {
    float cellvoltage = ( cgroup[itor].voltage * 7.8201 ) / 1000;
    Serial.print("Cell group ");
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

void readUnknown(void) {
  PACKET temppacket;
  int number;
  float packvoltage = 0;
  int result;

  for (int itor = 0; itor < 0x1F; itor++) {
    result = readPacket(0xC, temppacket); // 0xC and 0xCC are mirrored registers with unknown data
    if ( result == -1 ) {
      return;
    }
    if ( temppacket.mod == 0xff ) {
      Serial.println("COMMUNICATION ERROR DETECTED - CHECKSUM INVALID");
      return;
    }
    Serial.print("checksum [0x");
    Serial.print(temppacket.chk,HEX);
    Serial.print("] ");
    Serial.print("msb [0x");
    Serial.print(temppacket.msb),HEX;
    Serial.print("] ");
    Serial.print("lsb [0x");
    Serial.print(temppacket.lsb,HEX);
    Serial.println("]");
  }
}

void introMessage(void) {
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
}

void doMenu(void) {
  Serial.println("V) Read raw cell group voltages");
  Serial.println("T) Read temperature sensors");
  Serial.println("S) Read serial number");
  Serial.println("U) Read unknown data from register 0xC/0xCC");
  Serial.println("");
  Serial.println("Press key to select menu item:");
  for (;;) {
    if (Serial.available() > 0) {
      int inByte = Serial.read();
      switch (inByte) {
        case 'V': readVoltages(); break;
        case 'v': readVoltages(); break;
        case 'T': readTemps(); break;
        case 't': readTemps(); break;
        case 'S': readSerialNumber(); break;
        case 's': readSerialNumber(); break;
        case 'U': readUnknown(); break;
        case 'u': readUnknown(); break;
        default: continue;
      }
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
