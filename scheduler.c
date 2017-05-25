#include "scheduler.h"

void init_scheduler() {
	sch_storage = (scheduler_item_t *) malloc( sizeof(scheduler_item_t) * MAX_SCHTASKS );
}
