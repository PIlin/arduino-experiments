
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
	static unsigned long last_send = millis();
	static unsigned long next_send = 11000;
	if (false && (millis() - last_send > next_send))
	{
		xbee.send_data("ZZZZ", 4);

		next_send = random(8000, 150000);
		Serial.print("next_send ");
		Serial.println(next_send);
	}

	if (true && (millis() - last > PERIOD))
	{
		xbee.check_command_mode_exit_timeout(false);

		XBee::Command c("ID");
		xbee.send_command(c);
		char buf[30];
		uint8_t len = xbee.read_response_str(buf, 30);

		Serial.println();
		Serial.print("ATID = ");
		Serial.println(buf);

		xbee.stop_command_mode();
		last = millis();
	}

	echo();
}

