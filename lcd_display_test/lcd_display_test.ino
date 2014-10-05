#include <LiquidCrystal.h>
#include <Adafruit_MCP23008.h>

/*
 Demonstration sketch for Adafruit i2c/SPI LCD backpack
 using MCP23008 I2C expander
 ( http://www.ladyada.net/products/i2cspilcdbackpack/index.html )
 
 This sketch prints "Hello World!" to the LCD
 and shows the time.
 
 The circuit:
 * 5V to Arduino 5V pin
 * GND to Arduino GND pin
 * CLK to Analog #5
 * DAT to Analog #4
 */

// include the library code:
#include "Wire.h"

// Connect via i2c, default address #0 (A0-A2 not jumpered)
LiquidCrystal lcd(0);

//only lowest 5 bits used
//5 x 8 character matrix
uint8_t HEART_CHARACTER[] = {
	0b00000000,
        0b00001010,
        0b00011111,
        0b00011111,
        0b00001110,
        0b00000100,
        0b00000000,
        0b00000000};

void setup() {
  // set up the LCD's number of rows and columns: 
  lcd.createChar(0, HEART_CHARACTER);
  lcd.begin(16, 2);
  lcd.clear();
  // Print a message to the LCD.
  lcd.print("HackRVA rocks!");

  lcd.setBacklight(HIGH);
}

void loop() {
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  lcd.setCursor(0, 1);
  // print the number of seconds since reset:
  lcd.print(millis()/1000); 

  delay(1000);
}


