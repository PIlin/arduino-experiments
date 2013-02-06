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

#include "XBee/XBee.h"

#define DEBUG 1
#include "debug.h"

/*
This example is for Series 2 XBee
Receives a ZB RX packet and sets a PWM value based on packet data.
Error led is flashed if an unexpected packet is received
*/

#ifdef __AVR_ATmega32U4__
# define XBEE_SERIAL Serial1
#else
# include <SoftwareSerial.h>
SoftwareSerial soft_serial(10, 11);
# define XBEE_SERIAL soft_serial
#endif

XBee xbee = XBee();

int statusLed = 13;
int errorLed = 13;
int dataLed = 13;

void flashLed(int pin, int times, int wait) {

    for (int i = 0; i < times; i++) {
      digitalWrite(pin, HIGH);
      delay(wait);
      digitalWrite(pin, LOW);

      if (i + 1 < times) {
        delay(wait);
      }
    }
}

// serial high
uint8_t shCmd[] = {'S','H'};
// serial low
uint8_t slCmd[] = {'S','L'};
// association status
uint8_t assocCmd[] = {'A','I'};

AtCommandRequest atRequest = AtCommandRequest(shCmd);

AtCommandResponse atResponse = AtCommandResponse();


void sendAtCommand() {
  Serial.println("Sending command to the XBee");

  // send the command
  xbee.send(atRequest);

  // wait up to 5 seconds for the status response
  if (xbee.readPacket(5000)) {
    // got a response!

    // should be an AT command response
    if (xbee.getResponse().getApiId() == AT_COMMAND_RESPONSE) {
      xbee.getResponse().getAtCommandResponse(atResponse);

      if (atResponse.isOk()) {
        Serial.print("Command [");
        Serial.print(atResponse.getCommand()[0]);
        Serial.print(atResponse.getCommand()[1]);
        Serial.println("] was successful!");

        if (atResponse.getValueLength() > 0) {
          Serial.print("Command value length is ");
          Serial.println(atResponse.getValueLength(), DEC);

          Serial.print("Command value: ");

          for (int i = 0; i < atResponse.getValueLength(); i++) {
            Serial.print(atResponse.getValue()[i], HEX);
            Serial.print(" ");
          }

          Serial.println("");
        }
      }
      else {
        Serial.print("Command return error code: ");
        Serial.println(atResponse.getStatus(), HEX);
      }
    } else {
      Serial.print("Expected AT response but got ");
      Serial.print(xbee.getResponse().getApiId(), HEX);
    }
  } else {
    // at command failed
    if (xbee.getResponse().isError()) {
      Serial.print("Error reading packet.  Error code: ");
      Serial.println(xbee.getResponse().getErrorCode());
    }
    else {
      Serial.print("No response from radio");
    }
  }
}






void proc_xb_pack(int timeout = 0)
{
  if (timeout)
  {
    if (!xbee.readPacket(timeout))
    {
      return;
    }
  }
  else
    xbee.readPacket();

  if (xbee.getResponse().isAvailable()) {
    // got something


    switch (xbee.getResponse().getApiId())
    {
      case ZB_RX_RESPONSE:
      {
        DEBUG_PRINTLN("ZB_RX_RESPONSE");

        static ZBRxResponse rx;
        xbee.getResponse().getZBRxResponse(rx);

        DEBUG_PRINTLN2H("ra64 msb ", rx.getRemoteAddress64().getMsb());
        DEBUG_PRINTLN2H("ra64 lsb ", rx.getRemoteAddress64().getLsb());
        DEBUG_PRINTLN2H("remadr16 ", rx.getRemoteAddress16());
        DEBUG_PRINTLN2H("option   ", rx.getOption());
        DEBUG_PRINTLN2H("data len ", rx.getDataLength());

        break;
      }
      case MODEM_STATUS_RESPONSE:
      {
        DEBUG_PRINTLN("MODEM_STATUS_RESPONSE");
        static ModemStatusResponse msr;
        xbee.getResponse().getModemStatusResponse(msr);

        DEBUG_PRINTLN2H("status", msr.getStatus());

        break;
      }
      case ZB_TX_STATUS_RESPONSE:
      {
        DEBUG_PRINTLN("ZB_TX_STATUS_RESPONSE");

        static ZBTxStatusResponse txsr;
        xbee.getResponse().getZBTxStatusResponse(txsr);

        DEBUG_PRINTLN2 ("success ", txsr.isSuccess());
        DEBUG_PRINTLN2H("rem adr ", txsr.getRemoteAddress());
        DEBUG_PRINTLN2H("del st  ", txsr.getDeliveryStatus());
        DEBUG_PRINTLN2H("disc st ", txsr.getDiscoveryStatus());
        DEBUG_PRINTLN2H("ret cnt ", txsr.getTxRetryCount());

        //was_tx_result = true;

        break;
      }
      case AT_COMMAND_RESPONSE:
      {
        DEBUG_PRINTLN("AT_COMMAND_RESPONSE");

        static AtCommandResponse resp;

        xbee.getResponse().getAtCommandResponse(resp);

        DEBUG_PRINT("command ");
        for (int i = 0; i < 2; ++i) DEBUG_PRINT((char)resp.getCommand()[i]);
        DEBUG_PRINTLN("");

        DEBUG_PRINTLN2H("status ", resp.getStatus());

        DEBUG_PRINT("value: ");
        for (int i = 0; i < resp.getValueLength(); ++i)
        {
          Serial.print(resp.getValue()[i], HEX);
          Serial.print(' ');
        }
        DEBUG_PRINTLN("");
        break;
      }
      default:
      {
        DEBUG_PRINTLN2("got api packet ", xbee.getResponse().getApiId());
        break;
      }
    }
  }
  else if (xbee.getResponse().isError())
  {
    DEBUG_PRINTLN2("Error reading packet.  Error code: ", xbee.getResponse().getErrorCode());
  }

}


void at_command(char const * c, uint8_t const* val = NULL, int size = 0, int timeout = 5000)
{
  uint8_t* p = (uint8_t*)c;

  static AtCommandRequest r;
  r.setCommand(p);
  if (val)
  {
    r.setCommandValue((uint8_t*)val);
    r.setCommandValueLength(size);
  }
  else
  {
    r.clearCommandValue();
  }

  xbee.send(r);
  proc_xb_pack(timeout);
}

void initial_xb_check()
{
  at_command("DH");
  at_command("DL");
  at_command("MY");
  at_command("SH");
  at_command("SL");
  // at_command("NP");
  // at_command("DD");
  at_command("CH");
  at_command("ID");
  // at_command("OP");
  // at_command("OI");
  // at_command("NO");
  // at_command("SC");
  // at_command("SD");
  // at_command("ZS");
  // at_command("NJ");
  // at_command("PL");
  // at_command("PM");
  // at_command("AP");
  // at_command("AO");
  // at_command("BD");
  // at_command("NB");
  // at_command("SB");
  // at_command("VR");
  // at_command("HV");
  // at_command("AI");
}

void initial_xb_setup()
{
  {
    uint8_t p[] = {0x0d};
    at_command("CH", p, sizeof(p));
  }

  {
    uint8_t p[] = {0x28, 0x42};
    at_command("ID", p, sizeof(p));
  }
}


void setup() {
  pinMode(statusLed, OUTPUT);
  pinMode(errorLed, OUTPUT);
  pinMode(dataLed,  OUTPUT);


  XBEE_SERIAL.begin(9600);
  xbee.begin(XBEE_SERIAL);


  while (!Serial);

  Serial.println("Hello from reader");

  initial_xb_setup();

  initial_xb_check();

  //flashLed(statusLed, 3, 50);
}

// continuously reads packets, looking for ZB Receive or Modem Status
void loop() {

  static unsigned long last_ai = 0;

  unsigned long now = millis();

  if (now - last_ai >= 10000)
  {
    last_ai = now;
    at_command("AI", NULL, 0, 0);
  }

  proc_xb_pack();

}