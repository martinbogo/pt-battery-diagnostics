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

typedef struct {
  unsigned char chk;
  unsigned char msb;
  unsigned char lsb;
  unsigned char ok;
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

#define I2C_DISPLAY
/* If you want to use an LCD I2C display, make sure you have the
   LiquidCrystal_I2C library installed and enabled in your Arduino
   installation
*/
#ifdef I2C_DISPLAY
#include <LiquidCrystal_I2C.h>
#include "char.h"
#define DISPADDR 0x27
#define DISPCOLS 20
#define DISPROWS 4

LiquidCrystal_I2C lcd(DISPADDR, DISPCOLS, DISPROWS);

struct displaymsg {
  char line1[20];
  char line2[20];
  char line3[20];
  char line4[20];
};

struct displaymsg i2cdisplay;
int spinstate;
#endif

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

   0x6
   0xC 0xCC is unknown but may contain battery status information

*/

String serialnum;
long i2cDisplayRenderMillis;
long menuMillis;
long blinkMillis;
long i2cDisplayRenderInterval = 100;
long i2cDisplayBlinkInterval = 250;
long menuInterval = 200;

TEMP_SENSOR tempsensor[TSENSORS];
CGROUP cgroup[CELLGROUPS];
DEFPACKET dpacket[31];

int refreshDisplay;

void setup() {
  Wire.begin();        // join i2c bus (address optional for master)
  Serial.begin(115200);  // start serial for output
#ifdef I2C_DISPLAY
  lcd.init();
  lcd.backlight();
  lcd.clear();
#endif
  introMessage();
  showMenu();
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - menuMillis > menuInterval ) {
    menuMillis = currentMillis;
    doMenuInput();
  }

#ifdef I2C_DISPLAY
  if (currentMillis - i2cDisplayRenderMillis > i2cDisplayRenderInterval ) {
    i2cDisplayRenderMillis = currentMillis;
    updateDisplay();
  }

  if (currentMillis - blinkMillis > i2cDisplayBlinkInterval ) {
    blinkMillis = currentMillis;
    doBlink();
  }
#endif  

}

int readPacket(int regval, PACKET &packet) {
  unsigned char ok;
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
    packet.ok = 0xff;
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
    packet.ok = (packet.chk + packet.msb + packet.lsb);
    Serial.print("DEBUG :");
    Serial.print(packet.ok, DEC);
    Serial.print(", ");
    Serial.println(packet.ok % 8, DEC);
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
    if ( temppacket.ok == 0xff ) {
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
    if ( temppacket.ok == 0xff ) {
      Serial.println("COMMUNICATION ERROR DETECTED - CHECKSUM INVALID");
      return;
    }
    Serial.print("checksum [0x");
    Serial.print(temppacket.chk, HEX);
    Serial.print("] ");
    Serial.print("msb [0x");
    Serial.print(temppacket.msb, HEX);
    Serial.print("] ");
    Serial.print("lsb [0x");
    Serial.print(temppacket.lsb, HEX);
    Serial.print("] ");
    Serial.print("ASCII [");
    Serial.write(temppacket.lsb);
    Serial.println("]");
  }
}

void readEveryRegister(void) {
  PACKET temppacket;
  int number;
  float packvoltage = 0;
  int result;
  char buf[3];

  for (int itor = 0; itor <= 0xFF; itor++) {
    // do it 4 times, just to check a theory
    for (int ytor = 0; ytor < 37; ytor++) {
      memset(&temppacket, 0, sizeof(temppacket));
      result = readPacket(itor, temppacket); // 0xC and 0xCC are mirrored registers with unknown data
      if ( result == -1 ) {
        Serial.print("Packet: ");
        Serial.print(itor, HEX);
        Serial.println(", PACKET ERROR");
#ifdef I2C_DISPLAY
        memset(i2cdisplay.line1, 0, sizeof(i2cdisplay.line1));
        memset(i2cdisplay.line2, 0, sizeof(i2cdisplay.line2));
        memset(i2cdisplay.line3, 0, sizeof(i2cdisplay.line3));
        memset(i2cdisplay.line4, 0, sizeof(i2cdisplay.line4));
        strcat(i2cdisplay.line1, "Packet: ");
        strcat(i2cdisplay.line1, itoa(itor, buf, 3));
        strcat(i2cdisplay.line1, ", PKT ERR");
        refreshDisplay = 1;
#endif
        return;
      }
      if ( temppacket.ok == 0xff ) {
        Serial.print(itor, HEX);
        Serial.println(" COMMUNICATION ERROR DETECTED");
#ifdef I2C_DISPLAY
        memset(i2cdisplay.line1, 0, sizeof(i2cdisplay.line1));
        memset(i2cdisplay.line2, 0, sizeof(i2cdisplay.line2));
        memset(i2cdisplay.line3, 0, sizeof(i2cdisplay.line3));
        memset(i2cdisplay.line4, 0, sizeof(i2cdisplay.line4));
        strcat(i2cdisplay.line1, "Packet: ");
        strcat(i2cdisplay.line1, itoa(itor, buf, 3));
        strcat(i2cdisplay.line1, ", PKT ERR");
        refreshDisplay = 1;
#endif
        return;
      }
      if ( temppacket.msb == 0x0 && temppacket.lsb == 0x2 ) {
        continue; // no interesting data at this register
      }
      Serial.print("address [");
      Serial.print(itor, DEC);
      Serial.print("] [0x");
      Serial.print(itor, HEX);
      Serial.print("] ");
      Serial.print("crc ok? [");
      Serial.print(temppacket.ok, HEX);
      Serial.print("] ");
      Serial.print("checksum [0x");
      Serial.print(temppacket.chk, HEX);
      Serial.print("] ");
      Serial.print("msb [0x");
      Serial.print(temppacket.msb, HEX);
      Serial.print("] ");
      Serial.print("lsb [0x");
      Serial.print(temppacket.lsb, HEX);
      Serial.print("] ");
      Serial.print("ASCII [");
      Serial.write(temppacket.lsb);
      Serial.println("]");
    }
  }
}
void introMessage(void) {
#ifdef I2C_DISPLAY
  strcpy(i2cdisplay.line1,"Segway Battery Diag");
  strcpy(i2cdisplay.line2,"V ");
  strcat(i2cdisplay.line2,VERSION);
#endif
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

void doMenuInput(void) {
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
      case 'R': readEveryRegister(); break;
      case 'r': readEveryRegister(); break;
      case 'H': showMenu(); break;
      case 'h': showMenu(); break;
      default: return;
    }
  }
}

void showMenu(void) {
  Serial.println("V) Read raw cell group voltages");
  Serial.println("T) Read temperature sensors");
  Serial.println("S) Read serial number");
  Serial.println("U) Read unknown data from register 0xC/0xCC");
  Serial.println("R) Read all registers once");
  Serial.println("H) Help! Show this menu");
  Serial.println("");
  Serial.println("Press key to select menu item:");
}

void printBits(byte myByte) {
  for (byte mask = 0x80; mask; mask >>= 1) {
    if (mask  & myByte)
      Serial.print('1');
    else
      Serial.print('0');
  }
}

#ifdef I2C_DISPLAY
void doBlink(void) {
  switch (spinstate) {
    case 0:
      lcd.createChar(0,batt0);
      lcd.setCursor(19,3);
      lcd.write(byte(0));
      spinstate++;
      break;
    case 1:
      lcd.createChar(0,batt1);
      lcd.setCursor(19,3);
      lcd.write(byte(0));
      spinstate++;
      break;
    case 2:
      lcd.createChar(0,batt2);
      lcd.setCursor(19,3);
      lcd.write(byte(0));
      spinstate++;
      break;
    case 3:
      lcd.createChar(0,batt3);
      lcd.setCursor(19,3);
      lcd.write(byte(0));
      spinstate++;
      break;
    case 4:
      lcd.createChar(0,batt4);
      lcd.setCursor(19,3);
      lcd.write(byte(0));
      spinstate++;
      break;
    case 5:
      lcd.createChar(0,batt5);
      lcd.setCursor(19,3);
      lcd.write(byte(0));
      spinstate++;
      break;
     case 6:
      lcd.createChar(0,batt6);
      lcd.setCursor(19,3);
      lcd.write(byte(0));
      spinstate++;
      break;
    case 7:
      lcd.createChar(0,batt7);
      lcd.setCursor(19,3);
      lcd.write(byte(0));
      spinstate=0;
      break;
    default:
      spinstate=0;
      break;
  }
}

void updateDisplay(void) {
  if (refreshDisplay) {
    lcd.clear();
    refreshDisplay = 0;
  }
  lcd.setCursor(0, 0);
  lcd.print(i2cdisplay.line1);
  lcd.setCursor(0, 1);
  lcd.print(i2cdisplay.line2);
  lcd.setCursor(0, 2);
  lcd.print(i2cdisplay.line3);
  lcd.setCursor(0, 3);
  lcd.print(i2cdisplay.line4);
}
#endif
