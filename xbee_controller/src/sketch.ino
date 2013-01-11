
//#include "XBee/XBee.h"

#define LED_PIN 13

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

    Serial.begin(9600);

    nblink(3, 100, 100);
}


void loop()
{
	static char buf[32];

	static int state = 0;
	static int i = 0;

	switch (state)
	{
	case 0:
	{
		nblink(2, 50, 100);
		delay(1000);
		//Serial.print("+++");
		for (int i = 0; i < 3; ++i)
		{
			Serial.print('+');
			delay(200);
		}

		state = 1;
		i = 0;
	}
	break;

	case 1:
	{
		while (Serial.available() > 0)
		{
			buf[i++] = Serial.read();

			if (i == 2)
			{
				if (buf[0] == 'O' && buf[1] == 'K' && buf[2] == '\r')
				{
					state = 2;
					break;
				}
			}
		}

	}
	break;

	case 2:
	{
		blink(300, 100);
	}

	}


}




/****/
/*
XBee xbee = XBee();

void setup()
{
	pinMode(LED_PIN, OUTPUT);

	xbee.begin(9600);
	nblink(3, 100, 100);
}

void loop()
{

}
*/