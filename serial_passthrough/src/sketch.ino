
#include <SoftwareSerial.h>

SoftwareSerial sserial(10, 11);

void setup()
{
	Serial.begin(57600);

	sserial.begin(9600);
}

void loop()
{
	if (sserial.available())
	{
		Serial.print((char)sserial.read());
	}


	if (Serial.available())
	{
		char c = Serial.read();
		sserial.print(c);
		// Serial.print(c);
	}
}

