#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include "opto22.h"


#define MAX_SCHTASKS 20

typedef struct sch_item {
	unsigned short	dmts;
	unsigned short	gate;
	o22_wood_t		wood;
	unsigned char *	next;
} scheduler_item_t;

extern scheduler_item_t * rootFill, tailFill;
extern scheduler_item_t * rootEmpty, tailEmpty;

extern scheduler_item_t * sch_storage;

#endif
