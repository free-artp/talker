#ifndef _GATES_H
#define _GATES_H

#include "opto22.h"

#define GATES_DEFAULT_DELAY 100	// usec

typedef struct gate {
	unsigned short num;
	unsigned short dmts_delay;
	unsigned short dmin;
	unsigned short dmax;
	unsigned short pindir;
	unsigned short dir;
	unsigned short pin;
	unsigned short delay;
	
} gate_t;

extern gate_t *gates;
extern int gates_num;

void gate_run( int gate_num);
gate_t * gates_choose_by_wood( o22_wood_t *wood);

//void gate_check(unsigned short dmts);

#endif
