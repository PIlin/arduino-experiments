
#include "fsm.h"
#include "Arduino.h"

#define DEBUG 1
#include "debug.h"

uint8_t get_class(fsm const* f, char c)
{
	uint8_t i;
	for (i = 0; i < f->classes_count; ++i)
	{
		if (c == f->classes[i])
			return i;
	}
	return f->classes_count;
}

uint8_t get_action(fsm const* f, uint8_t state, uint8_t newstate)
{
	uint8_t i;
	for (i = 0; i < f->actions_count; ++i)
	{
		if (f->actions[i].s == state && f->actions[i].n == newstate)
			return f->actions[i].a;
	}
	return FSM_NOTHING;
}

extern "C" int8_t fsm_parse(fsm const* f, unsigned long timeout, uint8_t parse_error)
{
	DEBUG_PRINTLN("fsm_parse");
	uint8_t state = 0;
	uint8_t newstate = 0;
	int8_t action;
	unsigned long begin_time = millis();
	while (1)
	{
		if (f->gen->avail(f->gen->avail_param))
		{
			char c = f->gen->next(f->gen->next_param);
			DEBUG_PRINTLN2("c = ", c);
			uint8_t cls = get_class(f, c);
			DEBUG_PRINTLN2("cls = ", cls);
			DEBUG_PRINTLN2("state = ", (int)newstate);
			if (cls != f->classes_count)
				newstate = f->table[state * f->states_count + cls];
			else
				newstate = -1;

			DEBUG_PRINTLN2("newstate = ", (int)newstate);

			if (-1 == newstate)
			{
				newstate = 0;
				if (parse_error)
					return FSM_PARSE_ERR;
			}
			action = get_action(f, state, newstate);
			DEBUG_PRINTLN2("action = ", action);
			if (action != FSM_NOTHING)
			{
				DEBUG_PRINTLN("WTF");
				return action;
			}

			if (millis() - begin_time > timeout)
				return FSM_TIMEOUT;

			state = newstate;
		}
	}
}