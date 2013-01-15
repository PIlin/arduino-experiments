
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



int ledPin = 13;

XBee xbee = XBee();

XBeeAddress64 addr64 = XBeeAddress64(0x00, 0xffff);
ZBTxRequest zbTx = ZBTxRequest(addr64, NULL, 0);
ZBTxStatusResponse txStatus = ZBTxStatusResponse();

///

struct point_t
{
  uint8_t x;
  uint8_t y;
};

static const uint8_t children_count = 4;
struct child
{
  uint32_t addr;
  uint8_t p;
};
static child children[children_count] =
{
  {0xE9795D40, 0},
  {0xDC5C2D40, 1},
  {0xD95C2D40, 2},
  {0xDA795D40, 3}
};

static uint32_t addr = 0xE9795D40;
static const uint8_t positions_count = 4;
static point_t positions[positions_count] = {
  {10, 10},
  {15, 169},
  {200, 215},
  {140, 25}
};


void change_pattern()
{
  // static uint8_t cur = 4;

  // ++cur;
  // cur = cur % positions_count;

  // DEBUG_PRINTLN2("next positions = ", cur);

  struct packet {
    uint8_t magic;
    uint8_t updates;
    struct info_t{
      uint32_t addr;
      point_t pos;
    } info[];
  };


  uint8_t buf[2 + sizeof(packet::info_t) * children_count];
  packet& p = *(packet*)buf;

  p.magic = 0x42;
  p.updates = children_count;

  for (int i = 0; i < children_count; ++i)
  {
    packet::info_t& info = p.info[i];

    info.addr = children[i].addr;

    children[i].p++;
    children[i].p %= positions_count;

    info.pos = positions[children[i].p];
  }


  zbTx.setPayload(buf);
  zbTx.setPayloadLength(sizeof(buf));


  DEBUG_PRINTLN("payload");
  for (int i = 0; i < zbTx.getPayloadLength(); ++i)
  {
    Serial.print(zbTx.getPayload()[i], HEX);
  }
  DEBUG_PRINTLN();

  xbee.send(zbTx);

  DEBUG_PRINTLN("send done");
}

//

void process_xbee_packets()
{
  xbee.readPacket();

  if (xbee.getResponse().isAvailable())
  {
    switch (xbee.getResponse().getApiId())
    {
    // case ZB_TX_STATUS_RESPONSE:
    //   {
    //     DEBUG_PRINTLN("ZB_TX_STATUS_RESPONSE");

    //     xbee.getResponse().getZBRxResponse(rx);
    //     process_ZB_RX_RESPONSE(rx);
    //     break;
    //   }
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
  pinMode(ledPin, OUTPUT);

  Serial.begin(9600);

  XBEE_SERIAL.begin(9600);
  xbee.begin(XBEE_SERIAL);


  while (!Serial);

  Serial.println("Greetings");
}

void loop()
{

  if (Serial.available() > 0)
  {
    char r = Serial.read();
    // DEBUG_PRINTLN2("serial available", r);

    change_pattern();
  }

  process_xbee_packets();
}
