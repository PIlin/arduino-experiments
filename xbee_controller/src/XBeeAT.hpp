#ifndef XBeeAT_HPP__
#define XBeeAT_HPP__


#include <Arduino.h>

// class Stream;

#define TYPED_VAR(type, val) ((type)val)

#define ULONG_MAX TYPED_VAR(unsigned long, 0xFFFFFFFF)


#define DEBUG 1

#if DEBUG
# define DEBUG_PRINT(x)   do { Serial.print(x);   } while (0)
# define DEBUG_PRINTLN(x) do { Serial.println(x); } while (0)
#else
# define DEBUG_PRINT(x)
# define DEBUG_PRINTLN(x)
#endif

namespace XBee
{

#define COMMAND_BUF_LEN 20
#define CR ((char)0x0D)

class Command
{
public:
	Command(char const* const command) :
		command(command),
		param(NULL),
		param_length(0)
	{
	}

	// Command(char const* const command, int param) :
	// 	command(command)
	// {
	// }

private:
	char const* const command;
	char const* const param;
	uint8_t const param_length;

	friend class XBee;
};

unsigned long parse_hex(char const* const buf, uint8_t const len)
{
	unsigned long res = 0;

	for (uint8_t i = 0; i < len; ++i)
	{
		char c = buf[i];

		if (c > 'F') c -= 'a' - 'A';
		if (c < '0' || c > 'F') return 0;
		if ('9' < c && c < 'A') return 0;

		c = (c >= 'A') ? (c - 'A' + 10) : (c - '0');

		res = (res * 16) + c;
	}

	return res;
}


class XBee
{
public:
	XBee(Stream* serial) :
		serial(serial),
		last_com_time(ULONG_MAX),
		last_send_time(ULONG_MAX),
		//com_mode_exit_timeout(0x64 * 100), // default value
		com_mode_enter_timeout(1000),      // default value
		com_mode(false)
	{
		set_com_mode_exit_timeout(0x64); // default value

	}

	void send_command(Command const& command)
	{
		check_command_mode(true);

		serial->print("AT");
		serial->print(command.command);
		for (uint8_t i = 0; i < command.param_length; ++i)
		{
			serial->write(command.param[i]);
		}
		last_com_time = millis();
		serial->write(CR);
	}

	void send_data(void const* data, uint8_t size)
	{
		check_command_mode(false);

		uint8_t const* p = reinterpret_cast<uint8_t const*>(data);

		while (size-- > 0)
		{
			serial->write(*p);
			++p;
		}

		last_send_time = millis();
	}

	void send_data(uint8_t const c)
	{
		check_command_mode(false);

		serial->write(c);

		last_send_time = millis();
	}

	int read()
	{
		return serial->read();
	}

	int available()
	{
		return serial->available();
	}

	uint8_t read_response(char* const buf, uint8_t const size)
	{
		uint8_t i = 0;
		for (; i < size;)
		{
			if (serial->available())
			{
				const uint8_t c = serial->read();
				if (c == CR)
					break;

				buf[i] = c;
				++i;
			}
		}

		return i;
	}

	bool read_ok(unsigned long const timeout)
	{
		unsigned long begin = millis();
		int state = 0;
		while (1)
		{
			if (serial->available())
			{
				char c = serial->read();
				switch (state)
				{
				case 0:
					if (c == 'O') state = 1;
					else state = 0;
					break;
				case 1:
					if (c == 'K') state = 2;
					else state = 0;
					break;
				case 2:
					if (c == CR) state = 3;
					else state = 0;
					break;
				default:;
				}

				if (state == 3)
				{
					last_com_time = millis();
					DEBUG_PRINTLN("read_ok true");
					return true;
				}

				if (state == -1)
				{
					DEBUG_PRINTLN("read_ok false");
					return false;
				}
			}

			if (millis() - begin > timeout)
			{
				DEBUG_PRINTLN("read_ok timeout");
				return false;
			}
		}
	}

	void stop_command_mode()
	{
		check_command_mode(false);
	}

private:

	uint8_t enter_com_mode(void)
	{
		unsigned long wait = millis() - last_send_time;

		DEBUG_PRINT("enter_com_mode ");
		DEBUG_PRINTLN(wait);

		if (wait < com_mode_enter_timeout)
			delay(com_mode_enter_timeout - wait);

		serial->print("+++");

		if (!read_ok(com_mode_enter_timeout + 5))
			return false;

		com_mode = true;
		DEBUG_PRINTLN("com_mode = true");
		return true;
	}

	void exit_com_mode(void)
	{
		DEBUG_PRINTLN("exit_com_mode");
		serial->print("ATCN");
		serial->write(CR);

		read_ok(com_mode_exit_timeout);

		com_mode = false;
		DEBUG_PRINTLN("com_mode = false");
	}

	void check_command_mode(const bool enter)
	{
		const unsigned long now = millis();

		com_mode = com_mode && (now - last_com_time < com_mode_exit_timeout);
		DEBUG_PRINT("check_command_mode ");
		DEBUG_PRINT(com_mode);
		DEBUG_PRINTLN(now - last_com_time);

		if (enter && !com_mode)
		{
			while (!enter_com_mode())
			{
				// in case when last send considered as not the command
				last_send_time = millis();
			}
		}
		else if (!enter && com_mode)
		{
			exit_com_mode();
		}
	}

private:

	void set_com_mode_exit_timeout(unsigned long val) { com_mode_exit_timeout = val*100; }

private:

	Stream* serial;
	unsigned long last_com_time;
	unsigned long last_send_time;
	unsigned long com_mode_exit_timeout;
	unsigned long com_mode_enter_timeout;
	bool com_mode;
};

}

#endif // XBeeAT_HPP__