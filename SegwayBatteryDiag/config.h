
// For Lithium Segway Batteries uncomment these lines
#define TYPE 0x31
#define CELLGROUPS 23
#define TSENSORS 4

// For NiMH Segway Batteries uncomment these lines
/* #define TYPE 0x62
 * #define CELLGROUPS 6
 * #define TSENSORS 4
 */

// If you want Serial menu and debug, uncomment this
#define SERIAL_SHELL

/* If you want to use an LCD I2C display, make sure you have the
 * LiquidCrystal_I2C library installed and enabled in your Arduino
 * installation
 */
//#define I2C_LCD_DISPLAY // uncomment if you are using an I2C LCD display

/*
 * If you want to use the 96x64 full color OLED display, make sure
 * you have the SSD1306 library installed and enabled in your
 * Arduino installation.  This code uses the Adafruit_SSD1331
 * Library. You may need to adjust the code if you are using a
 * different display.
 */
//#define SPI_OLED_DISPLAY // uncomment if you are using an SPI OLED display
