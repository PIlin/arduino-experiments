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


#ifdef __AVR_ATmega32U4__
# define XBEE_SERIAL Serial1
#else
# include <SoftwareSerial.h>
SoftwareSerial soft_serial(10, 11);
# define XBEE_SERIAL soft_serial
#endif


// create the XBee object
XBee xbee = XBee();


static AtCommandResponse at_response;

uint8_t payload[1];
int statusLed = 13;

static bool was_tx_result = true;
static bool connected = false;

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

  if (xbee.getResponse().isAvailable())
  {

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

        DEBUG_PRINTLN2H("status ", msr.getStatus());

        switch (msr.getStatus())
        {
          case 2:
            DEBUG_PRINTLN("connected");
            connected = true;
            break;
          case 3:
            connected = false;
            was_tx_result = true;
            break;
        }

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

        was_tx_result = true;

        break;
      }
      case AT_COMMAND_RESPONSE:
      {
        DEBUG_PRINTLN("AT_COMMAND_RESPONSE");

        xbee.getResponse().getAtCommandResponse(at_response);

        DEBUG_PRINT("command ");
        for (int i = 0; i < 2; ++i)
          DEBUG_PRINT((char)at_response.getCommand()[i]);
        DEBUG_PRINTLN();

        DEBUG_PRINTLN2H("status ", at_response.getStatus());

        DEBUG_PRINT("value: ");
        for (int i = 0; i < at_response.getValueLength(); ++i)
        {
          Serial.print(at_response.getValue()[i], HEX);
          Serial.print(' ');
        }
        DEBUG_PRINTLN();

        break;
      }
      default:
      {
        DEBUG_PRINTLN2H("got api packet ", xbee.getResponse().getApiId());
        break;
      }
    }
  }
  else if (xbee.getResponse().isError())
  {
    DEBUG_PRINTLN2("Error reading packet.  Error code: ", xbee.getResponse().getErrorCode());
  }

}

void send_packet()
{
  static uint8_t counter = 1;


  if (!connected)
    return;


  if (!was_tx_result)
  {

    unsigned long now = millis();
    static unsigned long last_send = now;

    if (now - last_send <= 5000)
    {
      return;
    }

    last_send = now;
    DEBUG_PRINTLN("transmit status timeout");
  }

  was_tx_result = false;

  DEBUG_PRINTLN2("send_packet ", counter);
  payload[0] = counter++;

  // static XBeeAddress64 addr64 = XBeeAddress64(0x00a21300, 0xE9795D40);
  // static XBeeAddress64 addr64 = XBeeAddress64(0x0013a200, 0x40608a5b);
  static XBeeAddress64 addr64 = XBeeAddress64(0x0013a200, 0x405d79e9);
  static ZBTxRequest txrq = ZBTxRequest();

  txrq.setPayload(payload);
  txrq.setPayloadLength(1);
  txrq.setAddress64(addr64);
  txrq.setAddress16(0xFFFE);

  xbee.send(txrq);
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
  at_command("AP");
  // at_command("AO");
  // at_command("BD");
  // at_command("NB");
  // at_command("SB");
  // at_command("VR");
  // at_command("HV");

  at_command("AI");
  if (at_response.getValueLength() > 0 && at_response.getValue()[0] == 0)
  {
    DEBUG_PRINTLN("send AI");
    connected = true;
  }
}

void initial_xb_setup()
{
  // {
  //   uint8_t p[] = {0x0d};
  //   at_command("CH", p, sizeof(p));
  // }

  {
    uint8_t p[] = {0x28, 0x42};
    at_command("ID", p, sizeof(p));
  }

}

void setup() {
  pinMode(statusLed, OUTPUT);

  XBEE_SERIAL.begin(9600);
  xbee.begin(XBEE_SERIAL);


  Serial.begin(9600);

  while (!Serial);

  Serial.println("Hi, Writers");


  initial_xb_setup();

  initial_xb_check();
}


void loop() {

  static unsigned long last_ai = 0;

  unsigned long now = millis();

  if (now - last_ai >= 5000)
  {
    last_ai = now;
    DEBUG_PRINTLN("send AI");
    at_command("AI", NULL, 0, 0);
  }

  send_packet();
  proc_xb_pack();
}
