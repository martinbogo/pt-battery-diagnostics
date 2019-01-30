/*
   Segway Battery Diagnostics
   by Martin Bogomolni <martinbogo@gmail.com>


   Created 15 Jan, 2019
   Updated 27 Jan, 2018

   v 1.20

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

// For Lithium Segway Batteries uncomment these lines
#define TYPE 0x31
#define CELLGROUPS 23
#define TEMPSENSORS 4
// For NiMH Segway Batteries uncomment these lines
//#define TYPE 0x62
//#define CELLGROUPS 6
//#define TEMPSENSORS 4

int curiousregs[4] = { 12, 204 };
int unknownregs[1] = { 198 };

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

void readRegs(void) {
  unsigned char chk[128];
  unsigned char msb[128];
  unsigned char lsb[128];
  for ( unsigned int itor = 0; itor < 31; itor++ ) {
    Wire.beginTransmission(TYPE);
    Wire.write(0xC);
    Wire.endTransmission();
    Wire.requestFrom(TYPE, 3);
    while (Wire.available()) {
      unsigned char a = Wire.read();
      unsigned char b = Wire.read();
      unsigned char c = Wire.read();
      chk[itor] = a;
      msb[itor] = b;
      lsb[itor] = c;
    }
  }
  for ( int u = 0; u < 31; u++ ) {
    printBits(chk[u]);
    Serial.print(" ");
    printBits(msb[u]);
    Serial.print(" ");
    printBits(lsb[u]);
    Serial.print(" ");
    Serial.print(chk[u], HEX);
    Serial.print(" ");
    Serial.print(msb[u], HEX);
    Serial.print(" ");
    Serial.print(lsb[u], HEX);
    Serial.println();
  }
}

void readTemps(void) {
  for (int i = 0; i < TEMPSENSORS; i++) {
    Wire.beginTransmission(TYPE);
    Wire.write(0x17); // 0x17 and 0xD7 hold the temp sensor values, mirrored registers!
    Wire.endTransmission();

    Wire.requestFrom(TYPE, 3);
    while (Wire.available()) {
      char t = Wire.read(); // throwaway checksum
      delay(5);
      char h = Wire.read(); // high byte
      delay(5);
      char l = Wire.read(); // low byte
      unsigned int sensornum = word(h, l) >> 12; // top 4 bits are sensor number
      unsigned int rawtemp = word(h, l) & 0xFFF; // mask off 12 bits ADC
      float ctemp = rawtemp * 0.0625; // Sensor -256C to +256C scaled by 4096
      Serial.print("Temp Sensor [");
      Serial.print(sensornum);
      Serial.print("] ");
      Serial.print("Temperature in deg C: ");
      Serial.println(ctemp);
    }
  }
}

void readVoltages(void) {
  long packvoltage = 0L;
  for (int i = 0; i < CELLGROUPS; i++) {
    Wire.beginTransmission(TYPE); // i2c device on battery is at address 0x31
    Wire.write(0x56); // Both 0x96 and 0x56 register hold the battery voltages, mirrored registers!
    Wire.endTransmission();

    Wire.requestFrom(TYPE, 3);
    while (Wire.available()) {
      char t = Wire.read(); // throwaway checksum
      delay(5);
      char h = Wire.read();
      delay(5);
      char l = Wire.read();
      delay(5);
      unsigned int battery = word(h, l) >> 11; // top 5 bits are cell group
      int voltage = word(h, l) & 0x3FF; // 10 bits of ADC
      if ( voltage == 0x3ff ) {
        voltage = -1;
      }; // 0x3FF means a completely dead group
      long cellvoltage = voltage * 7.820; // Scale 8000 milivolts by 1023 steps
      Serial.print("Cell Group [");
      Serial.print(battery);
      Serial.print("] ");
      Serial.print("Voltage millivolt: ");
      if ( voltage < 0 ) {
        Serial.println("DEAD");
        cellvoltage =  0;
      } else {
        Serial.println(cellvoltage);
      }
      packvoltage = packvoltage + (long)cellvoltage;
    }
  }
  Serial.print("Pack voltage: ");
  Serial.print((float)packvoltage / 1000);
  Serial.println(" V");
}

void readDebugVoltages(void) {
  for (int i = 0; i < CELLGROUPS; i++) {
    Wire.beginTransmission(TYPE); // i2c device on battery is at address 0x31
    Wire.write(0x56); // Both 0x96 and 0x56 registers hold the battery voltages, mirrored registers!
    Wire.endTransmission();

    Wire.requestFrom(TYPE, 3);
    while (Wire.available()) {
      unsigned int t = Wire.read(); // throwaway checksum
      delay(5);
      unsigned int h = Wire.read();
      delay(5);
      unsigned int l = Wire.read();
      delay(5);
      unsigned int voltage = ((h << 8) + l) & 0x7FF; // 11 bit AD for Voltage
      unsigned int battery = ((h << 8) + l) >> 11; // top 5 bits are cell group
      Serial.print("Cell Group [");
      Serial.print(battery);
      Serial.print("] ");
      Serial.print("Checksum byte binary/hex [0b");
      Serial.print(t, BIN);
      Serial.print("]/[");
      Serial.print(t, HEX);
      Serial.print("] MSB binary/hex [0b");
      Serial.print(h, BIN);
      Serial.print("]/[0x");
      Serial.print(h, HEX);
      Serial.print("], LSB binary/hex[0b");
      Serial.print(l, BIN);
      Serial.print("]/[0x");
      Serial.print(l, HEX);
      Serial.print("], Word binary [0b");
      Serial.print((h << 8) + l, BIN);
      Serial.print("] 10 bit voltage binary/decimal [0b");
      Serial.print(((h << 8) + l) & 0x3FF, BIN);
      Serial.print("]/[");
      Serial.print(((h << 8) + l) & 0x3FF);
      Serial.println("]");
    }
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
  Serial.println("D) Read raw cell group voltages with lots of debug Info");
  Serial.println("T) Read temperature sensors");
  Serial.println("Q) Query some interesting registers");
  Serial.println("");
  Serial.println("Press key to select menu item:");
  for (;;) {
    if (Serial.available() > 0) {
      int inByte = Serial.read();
      switch (inByte) {
        case 'V': readVoltages(); break;
        case 'v': readVoltages(); break;
        case 'D': readDebugVoltages(); break;
        case 'd': readDebugVoltages(); break;
        case 'T': readTemps(); break;
        case 't': readTemps(); break;
        case 'Q': readRegs(); break;
        case 'q': readRegs(); break;
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
