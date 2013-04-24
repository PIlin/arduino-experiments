
#include <Arduino.h>

#include <SPI.h>

#include "mrf24j.h"

#define xstr(x) str(x)
#define str(x) #x

#define DEBUG_PRINT(x)   do { Serial.print(x);   } while (0)
#define DEBUG_PRINTLN(x) do { Serial.println(x); } while (0)
#define DEBUG_PRINTLN2(x,y) do { DEBUG_PRINT(x); DEBUG_PRINTLN(y); } while (0)
#define DEBUG_PRINTLN2H(x,y) do { DEBUG_PRINT(x); Serial.println(y, HEX); } while (0)

#define DEBUG_SHORT(x) do { DEBUG_PRINTLN2H(#x " ", mrf.read_short(x)); } while (0)
#define DEBUG_LONG(x) do { DEBUG_PRINTLN2H(#x " ", mrf.read_long(x)); } while (0)

//////////////////////////////////

static const int pin_reset = 5;
// static const int pin_cs = 7;
static const int pin_cs = 7;
static const int pin_interrupt = 2;

static word addresses[] = {
	0x6001, 0x2842
};

static Mrf24j mrf(pin_reset, pin_cs, pin_interrupt);

static void mrf_interrupt(void)
{
	mrf.interrupt_handler();
}

static void handle_rx(void)
{
	Serial.println("handle_rx");

	Serial.print("received a packet ");
	Serial.print(mrf.get_rxinfo()->frame_length, DEC);
	Serial.println(" bytes long");

	if(mrf.get_bufferPHY()){
		Serial.println("Packet data (PHY Payload):");
		for (int i = 0; i < mrf.get_rxinfo()->frame_length; i++) {
			Serial.print(mrf.get_rxbuf()[i]);
		}
	}

	Serial.println("\r\nASCII data (relevant data):");
	for (int i = 0; i < mrf.rx_datalength(); i++) {
		Serial.write(mrf.get_rxinfo()->rx_data[i]);
	}

	Serial.print("\r\nLQI/RSSI=");
	Serial.print(mrf.get_rxinfo()->lqi, DEC);
	Serial.print("/");
	Serial.println(mrf.get_rxinfo()->rssi, DEC);
}

static void handle_tx(void)
{
	Serial.println("handle_tx");

	if (mrf.get_txinfo()->tx_ok) {
		Serial.println("TX went ok, got ack");
	} else {
		Serial.print("TX failed after ");
		Serial.print(mrf.get_txinfo()->retries);
		Serial.println(" retries\n");
	}
}

static void setup_mrf(void)
{


	Serial.println("setup mrf node " xstr(ARDUINO_NODE_ID));

	//SPI.setClockDivider(B00000001); // spi speed

	mrf.reset();
	mrf.init();

	mrf.set_pan(0x2842abba);
	mrf.address16_write(addresses[ARDUINO_NODE_ID]);

	//mrf.set_promiscuous(true);

	//mrf.write_short(MRF_RXMCR, 0x02); // error mode

	attachInterrupt(0, mrf_interrupt, CHANGE);
	interrupts();

	delay(1000);

	Serial.println("done setup mrf");
}

void check_mrf(void)
{
	DEBUG_PRINTLN("check_mrf");
	int pan = mrf.get_pan();
	DEBUG_PRINTLN2H("pan ", pan);

	int addr = mrf.address16_read();
	DEBUG_PRINTLN2H("addr ", addr);

	DEBUG_SHORT(MRF_PACON2);
	DEBUG_SHORT(MRF_TXSTBL);

	DEBUG_LONG(MRF_RFCON0);
	DEBUG_LONG(MRF_RFCON1);
	DEBUG_LONG(MRF_RFCON2);
	DEBUG_LONG(MRF_RFCON6);
	DEBUG_LONG(MRF_RFCON7);
	DEBUG_LONG(MRF_RFCON8);
	DEBUG_LONG(MRF_SLPCON1);

	DEBUG_SHORT(MRF_BBREG2);
	DEBUG_SHORT(MRF_CCAEDTH);
	DEBUG_SHORT(MRF_BBREG6);

	DEBUG_PRINTLN("done check_mrf");
}
void setup() {

	Serial.begin(57600);
	while (!Serial);

	setup_mrf();
	check_mrf();

}

static void do_tx()
{
	static unsigned long was = millis();
	unsigned long now = millis();
	if (now - was >= 1000)
	{
		was = now;
		DEBUG_PRINTLN2("do_tx ", now);

		mrf.send16(addresses[1 - ARDUINO_NODE_ID], "lorem ipsum");
	}

}

void loop() {
	static unsigned long was = millis();
	unsigned long now = millis();
	if (now - was >= 1000)
	{
		was = now;
		DEBUG_PRINTLN2("alive ", now);
	}

	mrf.check_flags(&handle_rx, &handle_tx);

#if ARDUINO_NODE_ID == 0
	do_tx();
#endif
}
