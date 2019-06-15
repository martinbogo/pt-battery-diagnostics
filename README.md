# i2-battery-diagnostics
**I2 Battery Diagnostics**

Arduino sketch, documentation for i2/x2 Segway Li-ION battery diagnostics ( also works with NiMH )

There have been a number of closed sourced solutions for battery diagnostics and management for the Segway I2/X2 and related batteries.  Many of these solutions have been sold with "per use" lockouts such as "battery gauges/revivers" which work around the battery saftey systems to bring low-voltage batteries back above 50V to allow charging.

Even simple diagnostics tools to read the temperatures, the four on-board thermocouples, the voltages and capacities of the parallel battery clusters in each pack, or just the serial number and battery revision can cost hundreds of dollars/euros.

This git repository is designed to open the world of Segway Battery management to everyone, make the cells understandable and useable by the greater community of battery recyclers, powerwall, e-vehicle creators, or just people who want to have a deeper understanding of the battery systems in their Segway PT/I2/X2.

Included here are resources to help you such as:

 - Arduino .ino sketch with a simple menu system to read real-time diagnostics information from the battery
 - A breadboard schematic as well as pinout diagram for the Arduino and Segway battery to connect safely to the battery

I'm working on:

 - A schematic and PCB layout in Eagle format, along with Gerbers and Excellon drill files for an example Arduino shield
 - A schematic and PCB layout in Eagle format for an Arduino Mega & Adafruit 3.5" TFT display + STL to 3D print a case
 - A schematic and PCB layout in Eagle format for a Raspberry Pi + Python code command line diag tool
 - A BOM ( Bill of Materials ) with sources and links to purchase parts 
 - An STL file to 3D print a compatible connector to the Segway battery and instructions on how to assemble and use it safely.
 - If you need a 3D printed battery connector made, I highly recommend the one Rotek designed here on Shapeways.  Well worth supporting his work! https://www.shapeways.com/product/EN759FWRC/segway-battery-conector 

For information, please contact me here on GitHub by creating an issue.  I generally respond within 1-2 days.

---

[ UPDATE ]

Jun 15, 2019

After a short pause, development continues!  With the help of the online community and a friend, we have identified the function of two registers!

Battery State of Charge ( Total percentage of charge ) is an 8-bit field located at 0x1D ( decimal 29 ) 

Battery charge current is located at 0xD4 ( decimal 212 ) .. divide the value by 0.128 to get actual amperage. 

Feb 28, 2019

Average pack temperature register has been identified -- it's 149/0x95 and follows the same format as the other temp sensor data of 12 bits from MSB->LSB scaled from -256C to +256C

Feb 27, 2019

I had an "AHA!" moment today as I was working on decoding some other registers in the Segway battery pack.  I got the algorithm for the checksum wrong, but only slightly so.

To calculate the checksum:

Sum the __REGISTER__ + Checksum byte + Most Significant Byte + Least Significant Byte + __1__

Then check against modulus 64

e.g.) When you are reading from register 3:

  - 3 + 121 + 0 + 3 + 1 = 128 mod 64 = 0

It's as easy as that.


Feb 25, 2019

I'm going to organize the directory structure a little better today and add my lab notes into a separate "docs" directory with notes on the register functions and packet structure.  The code is also getting refactored today in the master branch... look for the addition of a config.h file and a change in handing serial input from using a raw loop to using a millis() timing function so serial input can be mixed with i2c displays.

Based on feedback, I'm adding two types of i2c display:

 - Simple 20x4 character LCD display
 - Full color 0.96" OLED display
 
I am adding optional support for a 1-button (long-press menu, short press select ) and 5-button input matrix ( up,down,left,right,OK )   Expect this to be implemented somewhere around build v1.80.  


Feb 22, 2019

Since I started this project, the primary registers with "interesting data" for battery diagnostics were the ones that contained information such as cell group voltage, the four temperature sensors, and information such as the battery revision and serial number.

However! There are some other interesting single-read registers in the battery which contain infomation such as the state-of-charge, overall pack voltage, and overall pack temperature.  There may be other registers with interesting information such as pack discharge amperage, and pack capacity/remaining capacity.

A quick read through of the registers shows that if the MSB is 0x0, and the LSB is 0x2, that address likely does not contain any interesting info.

This leaves the following registers, which I just scanned for single values:

```
address [3] [0x3] modcheck [4] checksum [0x79] msb [0x0] lsb [0x3] ASCII []
address [5] [0x5] modcheck [2] checksum [0x4E] msb [0x4] lsb [0x28] ASCII [(]
address [6] [0x6] modcheck [1] checksum [0xAA] msb [0x0] lsb [0x4F] ASCII [O]
address [18] [0x12] modcheck [5] checksum [0x6D] msb [0x0] lsb [0x0] ASCII []
address [20] [0x14] modcheck [3] checksum [0x6B] msb [0x0] lsb [0x0] ASCII []
address [27] [0x1B] modcheck [4] checksum [0xE6] msb [0xFF] lsb [0xFF] ASCII [⸮]
address [29] [0x1D] modcheck [2] checksum [0xCA] msb [0xFF] lsb [0x19] ASCII []
address [65] [0x41] modcheck [6] checksum [0x99] msb [0x18] lsb [0xCD] ASCII [⸮]
address [66] [0x42] modcheck [5] checksum [0x4F] msb [0x17] lsb [0x17] ASCII []
address [75] [0x4B] modcheck [4] checksum [0x4E] msb [0x3] lsb [0x23] ASCII [#]
address [77] [0x4D] modcheck [2] checksum [0xD2] msb [0x20] lsb [0x0] ASCII []
address [78] [0x4E] modcheck [1] checksum [0x61] msb [0x90] lsb [0x0] ASCII []
address [83] [0x53] modcheck [4] checksum [0x94] msb [0x0] lsb [0xD8] ASCII [⸮]
address [85] [0x55] modcheck [2] checksum [0x1A] msb [0x1] lsb [0x4F] ASCII [O]
address [92] [0x5C] modcheck [3] checksum [0xE5] msb [0xFF] lsb [0xFF] ASCII [⸮]
address [129] [0x81] modcheck [6] checksum [0x99] msb [0x18] lsb [0xCD] ASCII [⸮]
address [130] [0x82] modcheck [5] checksum [0x4F] msb [0x17] lsb [0x17] ASCII []
address [139] [0x8B] modcheck [4] checksum [0x4E] msb [0x3] lsb [0x23] ASCII [#]
address [141] [0x8D] modcheck [2] checksum [0xD2] msb [0x20] lsb [0x0] ASCII []
address [142] [0x8E] modcheck [1] checksum [0x61] msb [0x90] lsb [0x0] ASCII []
address [147] [0x93] modcheck [4] checksum [0x94] msb [0x0] lsb [0xD8] ASCII [⸮]
address [149] [0x95] modcheck [2] checksum [0x1A] msb [0x1] lsb [0x4F] ASCII [O]
address [156] [0x9C] modcheck [3] checksum [0xE5] msb [0xFF] lsb [0xFF] ASCII [⸮]
address [195] [0xC3] modcheck [4] checksum [0x79] msb [0x0] lsb [0x3] ASCII []
address [197] [0xC5] modcheck [2] checksum [0x4E] msb [0x4] lsb [0x28] ASCII [(]
address [210] [0xD2] modcheck [5] checksum [0x6D] msb [0x0] lsb [0x0] ASCII []
address [212] [0xD4] modcheck [3] checksum [0x6B] msb [0x0] lsb [0x0] ASCII []
address [219] [0xDB] modcheck [4] checksum [0xE6] msb [0xFF] lsb [0xFF] ASCII [⸮]
address [221] [0xDD] modcheck [2] checksum [0xCA] msb [0xFF] lsb [0x19] ASCII []
```
Feb 21, 2019

There is one more register of information to decode.  The data lives at 0xC/0xCC, and there are 0x0->0x1E packets to be read from that address.   I have added and uploaded a menu item 'U' into the code to read this address, so that we can work on decoding the meaning of the values stored there.

It uses the same packet communication method -- reading triplets of data [check] [high] [low]

```
checksum [0xC3] msb [0x0] lsb [0xB0] ASCII [⸮]
checksum [0xF1] msb [0x1] lsb [0x1] ASCII []
checksum [0xBE] msb [0x2] lsb [0x33] ASCII [3]
checksum [0x3F] msb [0x3] lsb [0x31] ASCII [1]
checksum [0x37] msb [0x4] lsb [0x38] ASCII [8]
checksum [0x3A] msb [0x5] lsb [0x34] ASCII [4]
checksum [0xC] msb [0x6] lsb [0x61] ASCII [a]
checksum [0x34] msb [0x7] lsb [0x38] ASCII [8]
checksum [0xA8] msb [0x8] lsb [0x43] ASCII [C]
checksum [0x27] msb [0x9] lsb [0x43] ASCII [C]
checksum [0xA6] msb [0xA] lsb [0x43] ASCII [C]
checksum [0x25] msb [0xB] lsb [0x43] ASCII [C]
checksum [0x37] msb [0xC] lsb [0x30] ASCII [0]
checksum [0x33] msb [0xD] lsb [0x33] ASCII [3]
checksum [0xA3] msb [0xE] lsb [0x42] ASCII [B]
checksum [0xA2] msb [0xF] lsb [0x42] ASCII [B]
checksum [0x21] msb [0x10] lsb [0x42] ASCII [B]
checksum [0x20] msb [0x11] lsb [0x42] ASCII [B]
checksum [0x31] msb [0x12] lsb [0x30] ASCII [0]
checksum [0x2D] msb [0x13] lsb [0x33] ASCII [3]
checksum [0x3F] msb [0x14] lsb [0x20] ASCII [ ]
checksum [0x3E] msb [0x15] lsb [0x20] ASCII [ ]
checksum [0x3D] msb [0x16] lsb [0x20] ASCII [ ]
checksum [0x3C] msb [0x17] lsb [0x20] ASCII [ ]
checksum [0xBB] msb [0x18] lsb [0x20] ASCII [ ]
checksum [0xBA] msb [0x19] lsb [0x20] ASCII [ ]
checksum [0xB9] msb [0x1A] lsb [0x20] ASCII [ ]
checksum [0x83] msb [0x1B] lsb [0x55] ASCII [U]
checksum [0x82] msb [0x1C] lsb [0x55] ASCII [U]
checksum [0xD5] msb [0x1D] lsb [0x81] ASCII [⸮]
checksum [0xDC] msb [0x1E] lsb [0xF9] ASCII [⸮]
```


Feb 21, 2019

The serial number has been located! ~~The checksum algorithm has been decoded!~~  Here's the quick summary:

For registers that are read as triplets ( temp / voltage / serial number ) a 3-byte packet is read in the following order:

[checkum] [ most significant byte ] [ least significant byte ]

~~In order to make sure the packet was transmitted properly, the checksum algorithm is simple -- add the three values up, and take the modulus 8 of the sum.  If the value is equal to 1, the packet is valid.~~

~~i.e.: ~~

~~[ 2D ] + [ 1C ] + [ 30 ] = 0x79~~
~~0x79 & 8 = 1~~

~~The packet is valid and was transmitted properly.~~

__NOTE__ **See my update on Feb 27th!  The correct algorithm to decode the checksum is there**

I am rewriting the code with a generic packet reader, which will check for i2c bus errors and the CRC, then return a PACKET struct containing the calculated checksum, and the three bytes ( everything reads 0xFF if there was an error )

-- 

The serial number is in plaintext ASCII, located at register 0xC6 (198)

Revision are the bytes stored in 0xC-0xD [ AH in the sample data below ]
Serial number are the bytes stored in 0x15-0x20 [ C01061400098 in the data below ]

```
10101010 00000000 01001111 AA 0 4F
11110111 00000001 00000001 F7 1 1
01000101 00000010 00110010 45 2 32
01000110 00000011 00110000 46 3 30
00111100 00000100 00111001 3C 4 39
00111110 00000101 00110110 3E 5 36
00111100 00000110 00110111 3C 6 37
01000010 00000111 00110000 42 7 30
01000001 00001000 00110000 41 8 30
01000000 00001001 00110000 40 9 30
10111111 00001010 00110000 BF A 30
00111101 00001011 00110001 3D B 31
00101100 00001100 01000001 2C C 41 __A__
00100100 00001101 01001000 24 D 48 __H__
10111010 00001110 00110001 BA E 31
10111010 00001111 00110000 BA F 30
00111000 00010000 00110001 38 10 31
00111000 00010001 00110000 38 11 30
10101110 00010010 00111001 AE 12 39
00110010 00010011 00110100 32 13 34
00101110 00010100 00110111 2E 14 37
10100001 00010101 01000011 A1 15 43 __C__
00110011 00010110 00110000 33 16 30 __0__
10110001 00010111 00110001 B1 17 31 __1__
00110001 00011000 00110000 31 18 30 __0__
10101010 00011001 00110110 AA 19 36 __6__
10101110 00011010 00110001 AE 1A 31 __1__
10101010 00011011 00110100 AA 1B 34 __4__
00101101 00011100 00110000 2D 1C 30 __0__
00101100 00011101 00110000 2C 1D 30 __0__
10101011 00011110 00110000 AB 1E 30 __0__
00100001 00011111 00111001 21 1F 39 __9__
10100001 00100000 00111000 A1 20 38 __8__
10000011 00100001 01010101 83 21 55
00000010 00100010 01010101 2 22 55
01011110 00100011 11111000 5E 23 F8
00100111 00100100 10101110 27 24 AE
11010100 00100101 00000000 D4 25 0
```

Feb 5, 2019

The battery voltage and temperature sensor code has been decoded, and the code is merged into the master branch.  

The data in register 12/204 is being worked on now, and I have a couple people helping to decode the data found on register 198 as well.  If you have a theory or suggestion as to the data, please submit an issue here on GitHub, or just modify the code and submit a patch request.

Jan 28, 2019

The Segway battery exposes interesting data on the following registers on i2c when read in the same way as the battery voltage values ( 3 bytes read, discard first byte, data encoded on 2nd and 3rd byte ) 

Register 12 ( 0xC ) reads 31 triplets.  Byte 0 ( unknown ) , Byte 2 counts from Hex 0 -> 1E, Byte 3 ( unknown )
Register 204 ( 0xCC ) reads 31 triplets.  Byte 0 ( unknown ), Bute 2 counts from Hex 0 -> 1E, Byte 3 ( unknown )
1st and 3rd values repeat after 31 reads, both registers show the same data

Register 198 ( 0xC6 ) reads 37 triplets.  Byte 0 ( unknown ), Byte 2 counts from Hex 0 -> 25, Byte 3 ( unknown )
1st and 3rd values repeat after 37 reads

Example of the data - register number in <>, values in hex

<12> [C3:0:B0] [F1:1:1] [BE:2:33] [3F:3:31] [37:4:38] [3A:5:34] [C:6:61] [34:7:38] [A8:8:43] [27:9:43] [A6:A:43] [25:B:43] [37:C:30] [33:D:33] [A3:E:42] [A2:F:42] [21:10:42] [20:11:42] [31:12:30] [2D:13:33] [3F:14:20] [3E:15:20] [3D:16:20] [3C:17:20] [BB:18:20] [BA:19:20] [B9:1A:20] [83:1B:55] [82:1C:55] [D5:1D:81] [DC:1E:F9]

<204> [C:6:61] [34:7:38] [A8:8:43] [27:9:43] [A6:A:43] [25:B:43] [37:C:30] [33:D:33] [A3:E:42] [A2:F:42] [21:10:42] [20:11:42] [31:12:30] [2D:13:33] [3F:14:20] [3E:15:20] [3D:16:20] [3C:17:20] [BB:18:20] [BA:19:20] [B9:1A:20] [83:1B:55] [82:1C:55] [D5:1D:81] [DC:1E:F9] [C3:0:B0] [F1:1:1] [BE:2:33] [3F:3:31] [37:4:38] [3A:5:34]

<198> [AA:0:4F] [F7:1:1] [45:2:32] [46:3:30] [3C:4:39] [3E:5:36] [3C:6:37] [42:7:30] [41:8:30] [40:9:30] [BF:A:30] [3D:B:31] [2C:C:41] [24:D:48] [BA:E:31] [BA:F:30] [38:10:31] [38:11:30] [AE:12:39] [32:13:34] [2E:14:37] [A1:15:43] [33:16:30] [B1:17:31] [31:18:30] [AA:19:36] [AE:1A:31] [AA:1B:34] [2D:1C:30] [2C:1D:30] [AB:1E:30] [21:1F:39] [A1:20:38] [83:21:55] [2:22:55] [5E:23:F8] [27:24:AE] [D4:25:0]


Register 23 ( 0x17 ) reads 4 triplets.  Byte 0 ( unknown ), Byte 2 increments 0x1,0x11,0x21,0x31, byte 3 ( unknown )
Register 215 ( 0xD7 ) reads 4 triplets.  Byte 0 ( unknown ), Byte 2 increments 0x1,0x11,0x21,0x31, byte 3 ( unknown )
1st and 3rd values repeat after four reads - these are likely the temperature readings on two registers

<23> [7F:1:68] [F3:11:64] [4E:21:79] [4A:31:6D]

<215> [7F:1:68] [F3:11:64] [4D:21:7A] [4A:31:6D]

This data parses to the following binary:

```0000 0001 0110 1000
0001 0001 0110 0100
0010 0001 0111 1010
0011 0001 0110 1101
---- Measurement index nibble
     -------------- Possible A/D conversion of the temperature ( 12 bit, or 10 bit? )
```

And of course, registers 150 (0x96) and 86 (0x56) contain the battery voltage data encoded as:

Byte 0 (checksum) : Byte 1, Byte 2 are the MSB and LSB of a 16 bit integer.  Top 3 bits are the battery index.  Lower 10 bits are A/D converted battery voltage data, where each tick represents 8 volts / 1023 steps.


Jan 27, 2019

After debugging the code with @gmulberry's input and live experimentation by feeding arbitrary voltages to an opened battery, we have determined that it's likely a 10bit ADC and that the voltage scaling factor is 8V ( 8000mV ) divided by 1023 steps ( 10 bit ADC ) to calculate the value of the voltage on the pack.

The code has been changed to reflect this, switching variables to the 32-bit long type to handle the math.  Each step of the ADC represents (8000/1023) or ~7.820 milivolts.


Jan 26, 2019 

First check-in for the Arduino code!  You can read the voltages from the cell groups with the currently checked in code.  I'll be updating the code with more functions as I clean up my research code.  

Also added:

 - Screen snapshot of the i2c bus communication to the battery when reading the voltages
 - My notes on reading the voltages, and an explanation of how the protocol works
 - Updated pinout of the connector, with proper location labels and functions for all pins
   -![Connector Pinout](https://github.com/martinbogo/i2-battery-diagnostics/blob/master/segway_pinout.png)
 - A terrible photo of my messy desk, that shows how I wired up the battery.  Embarrasing, but useful...
   -![Messy Desk](https://github.com/martinbogo/i2-battery-diagnostics/blob/master/photo_of_my_messy_setup.png)


Jan 20, 2019

Thanks to Segway Nation Tours in Austin, TX - I once again have access to a supply of both good and red-light batteries to continue the development and research!  I have had about a month of downtime, so am catching up.  I now have an Arduino Uno connected to a battery and can read all 23 cells in the pack.  As it turns out, 12V is needed to "wake" the battery up and I have modifed the picture to reflect that.

Initial Arduino -> Segway Battery pinout : Only four wires are required
 
 - SCL -> SCL pin on Battery
 - SDA -> SDA pin on Battery
 - GND -> GND pin on Battery ( CAREFUL! Don't connect to Segway Battery POS terminal )
 - +12V -> BAT ENABLE on Battery ( You can use VIN if you use a 12V power supply for your Arduino )
