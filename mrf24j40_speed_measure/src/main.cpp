
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


#define PACKETS_TO_SEND 100
#define PACKET_SIZE	20

char packet[PACKET_SIZE];

#if ARDUINO_NODE_ID == 0

uint_fast16_t lost_packages;
uint_fast16_t sent_packages;
bool sent;

static void handle_rx(void)
{

}

static void handle_tx(void)
{
	sent = true;
	// DEBUG_PRINTLN("s");
	++sent_packages;
	if (!mrf.get_txinfo()->tx_ok)
		++lost_packages;
}

static inline void SEND()
{
	sent = false;
	mrf.send16(addresses[1 - ARDUINO_NODE_ID], packet, PACKET_SIZE);
	while (!sent)
		mrf.check_flags(&handle_rx, &handle_tx);
}

static void do_speedtest(void)
{
	delay(1000);

	packet[0] = 1;

	for (uint_fast8_t j = 1; j < PACKET_SIZE; ++j)
	{
		packet[j] = random(0, 256);
	}

	sent_packages = 0;
	lost_packages = 0;

	// DEBUG_PRINTLN("\n\nstart speedtest");

	unsigned long begin_time = millis();

	for (uint_fast16_t i = 0; i < PACKETS_TO_SEND - 1; ++i)
	{
		SEND();
	}
	packet[0] = 0;
	SEND();

	unsigned long end_time = millis();

	Serial.print(sent_packages); Serial.print(';');
	Serial.print(lost_packages); Serial.print(';');
	Serial.print(begin_time); Serial.print(';');
	Serial.print(end_time); Serial.print(';');
	Serial.print(end_time - begin_time); Serial.print('\n');
}

static void print_banner(void)
{
	Serial.println("sent;errors;begin;end;total;test_size;packet_size");
	Serial.print("0;0;0;0;0;");
	Serial.print(PACKETS_TO_SEND); Serial.print(';');
	Serial.print(PACKET_SIZE); Serial.print('\n');
}


#else

static /*volatile*/ bool got_last = true;
static /*volatile*/ uint_fast16_t received = 0;
static /*volatile*/ uint_fast16_t error_lenght = 0;
static /*volatile*/ unsigned long begin_time = 0;
static /*volatile*/ unsigned long end_time = 0;

static void handle_rx(void)
{
	if (!received)
		begin_time = millis();

	++received;

	if (mrf.rx_datalength() != PACKET_SIZE)
		++error_lenght;

	// DEBUG_PRINTLN2H("fb ", mrf.get_rxinfo()->rx_data[0]);

	if (mrf.get_rxinfo()->rx_data[0] == 0)
	{
		got_last = true;
		end_time = millis();
	}
}

static void handle_tx(void)
{

}

static void do_speedtest(void)
{
	if (got_last)
	{
		Serial.print(received); Serial.print(';');
		Serial.print(error_lenght); Serial.print(';');
		Serial.print(begin_time); Serial.print(';');
		Serial.print(end_time); Serial.print(';');
		Serial.print(end_time - begin_time); Serial.print('\n');

		received = 0;
		error_lenght = 0;
		begin_time = 0;
		end_time = 0;
		got_last = 0;
	}

	mrf.check_flags(&handle_rx, &handle_tx);
}

static void print_banner(void)
{
	Serial.println("received;errors;begin;end;total;test_size;packet_size");
	Serial.print("0;0;0;0;0;");
	Serial.print(PACKETS_TO_SEND); Serial.print(';');
	Serial.print(PACKET_SIZE); Serial.print('\n');
}

#endif // ARDUINO_NODE_ID == 0

static void setup_mrf(void)
{


	Serial.println("setup mrf node " xstr(ARDUINO_NODE_ID));

	//SPI.setClockDivider(B00000001); // spi speed

	mrf.reset();
	mrf.init();

	mrf.set_pan(0xabba);
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
	pinMode(ledPin, OUTPUT);

	Serial.begin(57600);
	while (!Serial);

	setup_mrf();
	check_mrf();

	print_banner();
}

// static void do_tx()
// {
// 	static unsigned long was = millis();
// 	unsigned long now = millis();
// 	if (now - was >= 1000)
// 	{
// 		was = now;
// 		DEBUG_PRINTLN2("do_tx ", now);

// 		mrf.send16(addresses[1 - ARDUINO_NODE_ID], "lorem ipsum");
// 	}

// }

void loop() {
	do_speedtest();
}
