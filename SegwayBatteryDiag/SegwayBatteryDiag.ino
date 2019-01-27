/*
 * Segway Battery Diagnostics
 * by Martin Bogomolni <martinbogo@gmail.com>
 * 
 * Created 15 Jan, 2019
 * 
 * This code is copyright 2019, and under the MIT License
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
 
#include <Wire.h>

// For Lithium Segway Batteries uncomment these lines
#define TYPE 0x31
#define CELLGROUPS 23
// For NiMH Segway Batteries uncomment these lines
//#define TYPE 0x62
//#define CELLGROUPS 6

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

bool readVoltages(void) {
  
  for (int i = 0; i < CELLGROUPS; i++) {
    Wire.beginTransmission(TYPE); // i2c device on battery is at address 0x31
    Wire.write(0x96); // 0x96 register holds the battery voltages
    Wire.endTransmission();

    Wire.requestFrom(TYPE, 3);
    while (Wire.available()) {
      char t = Wire.read(); // throwaway checksum
      char h = Wire.read();
      char l = Wire.read();
      unsigned int voltage = word(h, l);
      unsigned int battery = word(h, l);
      voltage = voltage & 0x0FFF;
      battery = battery >> 11;
      Serial.print("Cell Group [");
      Serial.print(battery);
      Serial.print("] ");
      Serial.print("Voltage is ");
      Serial.println((4.20 / 4096)*voltage);
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
  Serial.println("V) Read Voltages");
  Serial.println("");
  Serial.println("Press key to select menu item:");
  for (;;) {
    switch (Serial.read()) {
      case 'V': readVoltages(); break;
      case 'v': readVoltages(); break;
      default: continue;
    }
  }
}
