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

I'll be releasing a beta of the software and hardware schematics mid-January 2019.  For information, please contact me here on GitHub.  I generally respond within 1-2 days.

[ UPDATE ]

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

And of course, registers 150 (0x96) and 86 (0x56) contain the battery voltage data encoded as:

Byte 0 (checksum) : Byte 1, Byte 2 are the MSB and LSB of a 16 bit integer.  Top 3 bits are the battery index.  Lower 10 bits are A/D converted battery voltage data, where each tick represents 8 volts / 1023 steps.


Jan 27, 2019

After debugging the code with @gmulberry's input and live experimentation by feeding arbitrary voltages to an opened battery, we have determined that it's likely a 10bit ADC and that the voltage scaling factor is 8V ( 8000mV ) divided by 1023 steps ( 10 bit ADC ) to cealculate the value of the voltage on the pack.

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
