#ifndef FSM_H__
#define FSM_H__

#include "stdint.h"


#ifdef __cplusplus
extern "C"{
#endif

typedef struct
{
	uint8_t s;
	uint8_t n;
	int8_t a;
} fsm_action;

typedef struct
{
	int (*avail)(void* param);
	void* avail_param;
	int (*next)(void* param);
	void* next_param;
} fsm_generator;

typedef struct fsm
{
	uint8_t classes_count;
	uint8_t states_count;
	char const* const classes;
	uint8_t const* const table;
	uint8_t actions_count;
	fsm_action const * const actions;
	fsm_generator const* gen;
} fsm;

#define FSM_NOTHING   (-1)
#define FSM_TIMEOUT   (-2)
#define FSM_PARSE_ERR (-3)

int8_t fsm_parse(fsm const* f, unsigned long timeout, uint8_t parse_error);

#ifdef __cplusplus
} // extern "C"
#endif


#endif // FSM_H__