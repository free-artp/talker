#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include "opto22.h"
#include "gates.h"

#define MAX_SCHTASKS 20

#define SCH_OK 		 0
#define SCH_EOL 	-1
#define SCH_BADPTR	-2


#define SCH_LOCK 		1
#define SCH_NOLOCK 	0

typedef struct sch_item {
	struct sch_item *	next;
	o22_wood_t			wood;
	unsigned short		dmts_start;
	unsigned short		ngate;
} scheduler_item_t;

extern scheduler_item_t *rootFill;
extern scheduler_item_t *rootEmpty;
extern scheduler_item_t *sch_storage_head;
extern volatile int scheduler_list_busy;

void release_scheduler();
void init_scheduler();
int scheduler_addtask( unsigned short cur_dmts, gate_t *cur_gate, o22_wood_t *wood, int check_lock);
int scheduler_droptask(scheduler_item_t *t, int check_lock);

void scheduler_dump();

scheduler_item_t * scheduler_findtask_by_wood_index(unsigned short index);
void * scheduler_executor(void *arg);

#endif
