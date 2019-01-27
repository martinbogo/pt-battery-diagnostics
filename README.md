# i2-battery-diagnostics
**I2 Battery Diagnostics**

Arduino sketch, library, BOM, and Gerber files for i2/x2 Segway battery diagnostics

There have been a number of closed sourced solutions for battery diagnostics and management for the Segway I2/X2 and related batteries.  Many of these solutions have been sold with "per use" lockouts such as "battery revivers" which work around the battery lockouts.  Even simple diagnostics tools to read the temperatures, the four on-board thermocouples, the voltages and capacities of the 23 batteries in each pack, or just the serial number and battery revision can cost hundreds of dollars/euros!

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

Jan 26, 2019 

First check-in for the Arduino code!  You can read the voltages from the cell groups with the currently checked in code.  I'll be updating the code with more functions as I clean up my research code.  

Also added:

 - Screen snapshot of the i2c bus communication to the battery when reading the voltages
 - My notes on reading the voltages, and an explanation of how the protocol works
 - Updated pinout of the connector, with proper location labels and functions for all pins
 - A terrible photo of my messy desk, that shows how I wired up the battery.  Embarrasing, but useful...

Jan 20, 2019

Thanks to Segway Nation Tours in Austin, TX - I once again have access to a supply of both good and red-light batteries to continue the development and research!  I have had about a month of downtime, so am catching up.  I now have an Arduino Uno connected to a battery and can read all 23 cells in the pack.  As it turns out, only 5V is needed to "wake" the battery up and I have modifed the picture to reflect that.

Initial Arduino -> Segway Battery pinout : Only four wires are required
 
 - SCL -> SCL pin on Battery
 - SDA -> SDA pin on Battery
 - GND -> GND pin on Battery ( CAREFUL! Don't connect to Segway Battery POS terminal )
 - +5V -> BAT ENABLE on Battery
