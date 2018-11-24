# i2-battery-diagnostics
**I2 Battery Diagnostics**

Arduino sketch, library, BOM, and Gerber files for i2/x2 Segway battery diagnostics

There have been a number of closed sourced solutions for battery diagnostics and management for the Segway I2/X2 and related batteries.  Many of these solutions have been sold with "per use" lockouts in the case of tools like "battery revivers" which work around the battery lockouts or even simple diagnostics tools to read the temperatures, the four on-board thermocouples, the voltages and capacities of the 23 batteries in each pack, or just the serial number and battery revision.

This git repository is designed to open the world of Segway Battery management to all, and make the cells understandable and useable by the greater community of battery recyclers, powerwall and e-vehicle creators, or simply people who want to have a deeper understanding of the battery systems in their Segway i2/X2.  

Included here are:

 - Arduino .ino sketch with a simple menu system to read real-time diagnostics information from the battery
 - A breadboard schematic as well as pinout diagram for the Arduino and Segway battery to connect safely to the battery
 - A schematic and PCB layout in Eagle format, along with Gerbers and Excellon drill files for an example Arduino shield
 - A schematic and PCB layout in Eagle format for an Arduino Mega & Adafruit 3.5" TFT display + STL to 3D print a case
 - A schematic and PCB layout in Eagle format for a Raspberry Pi + Python code command line diag tool
 - A BOM ( Bill of Materials ) with sources to purchase parts 
 - An STL file to 3D print a compatible connector to the Segway battery and instructions on how to assemble and use it safely.

I'll be releasing a beta of the software and hardware schematics ( verion 0.8 ) mid-January 2019.  For information, please contact me here on GitHub.  I generally respond within 1-2 days.


