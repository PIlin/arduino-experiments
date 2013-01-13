
#include <SoftwareSerial.h>

#define LED_PIN 13

SoftwareSerial sserial(10, 11);


#include "XBeeAT.hpp"


XBee::XBee xbee(&sserial);


void blink(int t1, int t2)
{
	digitalWrite(LED_PIN, HIGH);
	delay(t1);
	digitalWrite(LED_PIN, LOW);
	delay(t2);
}

void nblink(int c, int t1, int t2)
{
	for (int i = 0; i < c; ++i) blink(t1, t2);
}


void setup()
{
    pinMode(LED_PIN, OUTPUT);

    Serial.begin(57600);

    sserial.begin(9600);

    // nblink(3, 100, 100);
}

void echo()
{
	if (xbee.available())
	{
		Serial.print((char)xbee.read());
	}


	if (Serial.available())
	{
		char c = Serial.read();
		xbee.send_data(c);
		// Serial.print(c);
	}
}

#define PERIOD 10000

void loop()
{
	static unsigned long last = millis();

	if (true && (millis() - last > PERIOD))
	{
		XBee::Command c("ID");
		xbee.send_command(c);
		char buf[30];
		uint8_t len = xbee.read_response(buf, 30);
		buf[len] = 0;

		Serial.println();
		Serial.print("ATID = ");
		Serial.println(buf);

		xbee.stop_command_mode();
		last = millis();
	}

	echo();
}

