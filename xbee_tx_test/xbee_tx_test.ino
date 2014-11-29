/** -*- mode: arduino -*-
 *
 * Copyright (c) 2014 Adrian L. Flanagan
 *
 * This file is part of xbee_tx_text.
 *
 * xbee_tx_test is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * xbee_tx_test is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with xbee_tx_test.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

// #include <stdint.h>
#include <XBee.h>
#include <LiquidCrystal.h>
#include <Adafruit_MCP23008.h>
// include the library code:
#include "Wire.h"

/*********************************************************
 * Globals
 *
 * Most global variables are global to reduce memory consumption,
 * function call overhead.
 ********************************************************/
#define LCD_WIDTH 16
#define LCD_HEIGHT 2
#define SIXTEEN_SPACES "                "
//maximum length of an error message
#define MAX_ERR_MSG LCD_WIDTH - 4

int successCount = 0;
int errorCount = 0;
char field2[9]; //buffer for second field on display

// Connect via i2c, default address #0 (A0-A2 not jumpered)
LiquidCrystal lcd(0);

// create the XBee object
XBee xbee = XBee();

uint8_t payload[] = { 
  0, 0 };

// SH + SL Address of receiving XBee
XBeeAddress64 addr64 = XBeeAddress64(0x0013a200, 0x403e0f30);
ZBTxRequest zbTx = ZBTxRequest(addr64, payload, sizeof(payload));
ZBTxStatusResponse txStatus = ZBTxStatusResponse();

int pin5 = 0;

int statusLed = 13;
int errorLed = 13;

/*********************************************************
 * setup() -- required initialization routine
 *
 * Initialize output LEDs, serial output, LCD display.
 *********************************************************/
void setup() {
  pinMode(statusLed, OUTPUT);
  pinMode(errorLed, OUTPUT);

  Serial.begin(9600);
  xbee.setSerial(Serial);

  lcd.begin(LCD_WIDTH, LCD_HEIGHT);
  lcd.clear();
  // Print a message to the LCD.
  lcd.print("Testing XBee");

  lcd.setBacklight(HIGH);
}

/*********************************************************
 * doMessage() -- Display a status string on 2nd half of
 *   top row of the LCD
 *
 * Param: msg: zero-terminated char array.
 *
 * Displays up to 8 characters from msg starting at position
 * 8 of the first row of the LCD. If length of msg is greater
 * than 8, displays "msg err " instead.
 **********************************************************/
void doMessage(const char *msg) {
  int i = 8 + strlen(msg); // first pos past end
  lcd.setCursor(8, 1); 
  if (strlen(msg) > 8) {
    lcd.print("msg err ");
  } 
  else {
    lcd.print(msg);
    while (i++ < 8) {
      lcd.print(" ");
    }
  }
}

/**************************************************
 * displayError() -- standard error message display
 *
 * Params:
 *   msg -- string (max of LCD width - 4 characters)
 *
 * Clears first line of display and displays "ERR: "
 * and the contents of msg.
 * If the string at msg is longer than maximum,
 * displays "ERR: MSG OVERFLW"
 **************************************************/
void displayError(const char *msg) {
  lcd.setCursor(0, 0);
  lcd.print(SIXTEEN_SPACES);
  if (strlen(msg) > MAX_ERR_MSG) {
    lcd.print("ERR: MSG OVRFLW");
  } else {
    lcd.print(msg);
  }
}
  
void displayQuarter(const int field, const char *msg) {
}
/**************************************************
 * showStats() -- Display transmit stats on LCD
 *
 * Params:
 *   success - number of successful tries
 *   errors - count of errors seen
 *
 * Displays succes count, a space, then the error count on the
 * second line of the LCD (starting at 0, end depends on values)
 **************************************************/
void showStats(const int success, const int errors) {
  char msgBuff[16];
  lcd.setCursor(0, 1);
  itoa(success, msgBuff, 10);
  lcd.print(msgBuff);
  lcd.print(" ");
  itoa(errors, msgBuff, 10);
  lcd.print(msgBuff);
}

/**************************************************
 * sendText() -- Send two characters to destination
 *
 * Returns: error code (0 => success)
 **************************************************/
int sendText(const char *msg) {   

  // break down 10-bit reading into two bytes and place in payload
  //  pin5 = analogRead(5);
  //  payload[0] = pin5 >> 8 & 0xff;
  //  payload[1] = pin5 & 0xff;
  payload[0] = msg[0];
  payload[1] = msg[1];
  
  xbee.send(zbTx);

  doMessage("sent??");

  // after sending a tx request, we expect a status response
  // wait up to two seconds for the status response
  if (xbee.readPacket(2000)) {
    // got a response!
    successCount++;
    // should be a znet tx status            	
    if (xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE) {
      xbee.getResponse().getZBTxStatusResponse(txStatus);

      // get the delivery status, the fifth byte
      if (txStatus.getDeliveryStatus() == SUCCESS) {
        // success.  time to celebrate
        doMessage("succ");
      } 
      else {
        // the remote XBee did not receive our packet. is it powered on?
        doMessage("no rep");
      }
    }
  } 
  else if (xbee.getResponse().isError()) {
    //nss.print("Error reading packet.  Error code: ");  
    //nss.println(xbee.getResponse().getErrorCode());
    doMessage("ERR: ");
    lcd.print(xbee.getResponse().getErrorCode());
    errorCount++;
  } 
  else {
    // local XBee did not provide a timely TX Status Response -- should not happen
    doMessage("no stat");
    errorCount++;
  }

  showStats(successCount, errorCount);
  return 0;
}

void loop() {
  successCount++;
  if (successCount > 9999) { successCount = 0; }
  //copy from string literal -- only time strcpy is safe
  strcpy(field2, "test"); 
  ltoa(successCount, &field2[4], 10);
  doMessage(field2);
  delay(5000);
}
