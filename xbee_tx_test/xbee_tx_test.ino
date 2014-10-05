
/**
 * Copyright (c) 2009 Andrew Rapp. All rights reserved.
 *
 * This file is part of XBee-Arduino.
 *
 * XBee-Arduino is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * XBee-Arduino is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with XBee-Arduino.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdint.h>
#include <SoftwareSerial.h>
#include <XBee.h>
#include <LiquidCrystal.h>
#include <Adafruit_MCP23008.h>
// include the library code:
#include "Wire.h"

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

SoftwareSerial serial(2, 3);

void setup() {
  pinMode(statusLed, OUTPUT);
  pinMode(errorLed, OUTPUT);

  serial.begin(19200);
  xbee.setSerial(serial);

  lcd.begin(16, 2);
  lcd.clear();
  // Print a message to the LCD.
  lcd.print("Transmitting...!");

  lcd.setBacklight(HIGH);
}

void doMessage(char *msg) {
  int pos = strlen(msg);
  int i;
  lcd.setCursor(8, 1); 
  if (strlen(msg) > 16) {
    lcd.print("msg error       ");
  } 
  else {
    lcd.print(msg);
    for (i= pos; i < 16; i++) {
      lcd.print(" ");
    }
  };
}


void showStats(int success, int errors) {
  char msgBuff[16];
  lcd.setCursor(0, 1);
  itoa(success, msgBuff, 10);
  lcd.print(msgBuff);
  lcd.print(" ");
  itoa(errors, msgBuff, 10);
  lcd.print(msgBuff);
}

int successCount = 0;
int errorCount = 0;
void loop() {   

  // break down 10-bit reading into two bytes and place in payload
  pin5 = analogRead(5);
  payload[0] = pin5 >> 8 & 0xff;
  payload[1] = pin5 & 0xff;

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
  delay(5000);
}


