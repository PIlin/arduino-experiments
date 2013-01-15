#include <stdlib.h>
#include <stdint.h>
#include <Arduino.h>
#include "XBee/XBee.h"

#define DEBUG 1
#include "debug.h"

static const int statusLed = 13;
static const int blinkLed = 13;
static uint8_t slCmd[] = {'S','L'};

static XBee xbee = XBee();
static XBeeResponse response = XBeeResponse();

static ZBRxResponse rx = ZBRxResponse();
static ModemStatusResponse msr = ModemStatusResponse();

static AtCommandRequest atRequest = AtCommandRequest();
static AtCommandResponse atResponse = AtCommandResponse();



///

static uint32_t sl = 0;

///

// 0 ok
// 1 command error
// 2 other error
uint8_t sendAtCommand(AtCommandResponse& resp, int timeout = 5000) {
  // Serial.println("Sending command to the XBee");

  // send the command
  xbee.send(atRequest);

  // wait up to 5 seconds for the status response
  if (xbee.readPacket(timeout)) {
    // got a response!

    // should be an AT command response
    if (xbee.getResponse().getApiId() == AT_COMMAND_RESPONSE) {
      xbee.getResponse().getAtCommandResponse(resp);

      if (resp.isOk()) {
        DEBUG_PRINT("Command [");
        DEBUG_PRINT(resp.getCommand()[0]);
        DEBUG_PRINT(resp.getCommand()[1]);
        DEBUG_PRINTLN("] was successful!");

        return 0;

        // if (resp.getValueLength() > 0) {
        //   Serial.print("Command value length is ");
        //   Serial.println(resp.getValueLength(), DEC);

        //   Serial.print("Command value: ");

        //   for (int i = 0; i < resp.getValueLength(); i++) {
        //     Serial.print(resp.getValue()[i], HEX);
        //     Serial.print(" ");
        //   }

        //   Serial.println("");
        // }
      }
      else {
        Serial.print("Command return error code: ");
        Serial.println(resp.getStatus(), HEX);

        return 1;
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

  return 2;
}


void get_our_sl()
{
  // DEBUG_PRINT("get_our_sl");

  atRequest.setCommand(slCmd);

  while (0 != sendAtCommand(atResponse))
  {
    // DEBUG_PRINTLN("get_our_sl attempt");
  }

  char * const begin = reinterpret_cast<char*>(atResponse.getValue());
  char * end = begin + atResponse.getValueLength();


  // DEBUG_PRINTLN2("result len = ", atResponse.getValueLength());

  // for (int i = 0; i < atResponse.getValueLength(); ++i)
  // {
  //   Serial.print((uint8_t)begin[i], HEX);
  // }
  // DEBUG_PRINTLN();

  sl = *reinterpret_cast<uint32_t*>(begin);

  DEBUG_PRINT("sl = ");
  Serial.println(sl, HEX);
}

///

struct BlinkPattern
{
  uint16_t th;
  uint16_t tl;
};

const uint8_t patterns_count = 4;
const BlinkPattern patterns[patterns_count] =
{
  {50, 50},
  {150, 50},
  {1000, 200},
  {500, 100}
};

uint8_t our_pattern = 0;

void perform_blink()
{
  unsigned long now = millis();
  static unsigned long change_time = 0;
  static bool low = true;

  BlinkPattern const& p = patterns[our_pattern];

  if (now - change_time >= (low ? p.tl : p.th))
  {
    low = !low;
    digitalWrite(blinkLed, low ? LOW : HIGH);
    change_time = now;
  }
}

void process_pos_update(uint32_t addr, uint8_t x, uint8_t y)
{
  DEBUG_PRINT("addr = ");
  Serial.println(addr, HEX);

  if (addr != sl)
  {
    DEBUG_PRINTLN("not our address, ignoring");
    return;
  }

  x = (x < 128) ? 0 : 1;
  y = (y < 128) ? 0 : 1;


  our_pattern = y * 2 + x;
}

///

void process_ZB_RX_RESPONSE(ZBRxResponse& rx)
{
  const uint8_t magic = 0x42;
  uint8_t const* data = rx.getData();
  uint8_t const length = rx.getDataLength();

  DEBUG_PRINTLN("payload");
  for (int i = 0; i < length; ++i)
  {
    Serial.print(data[i], HEX);
  }
  DEBUG_PRINTLN();


  struct point_t
  {
    uint8_t x;
    uint8_t y;
  };

  struct packet {
    uint8_t magic;
    uint8_t updates;
    struct info_t{
      uint32_t addr;
      point_t pos;
    } info[];
  };



  if (length < 2)
  {
    DEBUG_PRINTLN("data is too small");
    return;
  }

  if (data[0] != magic)
  {
    DEBUG_PRINTLN("evil magic");
    return;
  }

  packet const& p = *reinterpret_cast<packet const*>(data);

  if (length < 2 + sizeof(packet::info_t) * p.updates)
  {
    DEBUG_PRINTLN("data is too small");
    return;
  }

  for (uint8_t i = 0; i < p.updates; ++i)
  {
    packet::info_t const& info = p.info[i];
    process_pos_update(info.addr, info.pos.x, info.pos.y);
  }
}


void process_xbee_packets()
{
  xbee.readPacket();

  if (xbee.getResponse().isAvailable())
  {
    switch (xbee.getResponse().getApiId())
    {
    case ZB_RX_RESPONSE:
      {
        DEBUG_PRINTLN("ZB_RX_RESPONSE");

        xbee.getResponse().getZBRxResponse(rx);
        process_ZB_RX_RESPONSE(rx);
        break;
      }
    default:
      {
        DEBUG_PRINTLN2("Got packet with apiId", xbee.getResponse().getApiId());
        break;
      }
    } // swtich
  }
  else if (xbee.getResponse().isError())
  {
    DEBUG_PRINTLN2("Error reading packet.  Error code: ", xbee.getResponse().getErrorCode());
  }
}

///

void setup()
{
  pinMode(statusLed, OUTPUT);

  // start serial
  Serial1.begin(9600);
  xbee.begin(Serial1);

  Serial.begin(9600);

  // while (!Serial);

  get_our_sl();
}

void loop()
{
  perform_blink();
  process_xbee_packets();
}
