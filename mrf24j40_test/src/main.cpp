
#include <Arduino.h>

#include "mrf24j.h"

# define DEBUG_PRINT(x)   do { Serial.print(x);   } while (0)
# define DEBUG_PRINTLN(x) do { Serial.println(x); } while (0)
# define DEBUG_PRINTLN2(x,y) do { DEBUG_PRINT(x); DEBUG_PRINTLN(y); } while (0)
# define DEBUG_PRINTLN2H(x,y) do { DEBUG_PRINT(x); Serial.println(y, HEX); } while (0)

#define DEBUG_SHORT(x) do { DEBUG_PRINTLN2H(#x " ", mrf.read_short(x)); } while (0)
#define DEBUG_LONG(x) do { DEBUG_PRINTLN2H(#x " ", mrf.read_long(x)); } while (0)
//////////////////////////////////

static const int ledPin = 13;

static void blink(uint8_t count, const unsigned long t)
{
	while (count--)
	{
		digitalWrite(ledPin, HIGH);
		delay(t);
		digitalWrite(ledPin, LOW);
		delay(t);
	}
}

#define ERROR_BLINK(count) blink(count, 100)

//////////////////////////////////

static const int pin_reset = 5;
// static const int pin_cs = 7;
static const int pin_cs = 10;
static const int pin_interrupt = 3;

static Mrf24j mrf(pin_reset, pin_cs, pin_interrupt);

static void mrf_interrupt(void)
{
	mrf.interrupt_handler();
}

static void handle_rx(void)
{
	Serial.println("handle_rx");
}

static void handle_tx(void)
{
	Serial.println("handle_tx");
}

void setup() {
	pinMode(ledPin, OUTPUT);

	Serial.begin(57600);
	while (!Serial);

	Serial.println("setup mrf");

	mrf.reset();
	mrf.init();

	mrf.set_pan(0xabba);
	mrf.address16_write(0x6001);

	Serial.println("done setup mrf");

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
}



void loop() {
	mrf.check_flags(&handle_rx, &handle_tx);
}
