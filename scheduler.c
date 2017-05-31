#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>

#include "scheduler.h"
#include "info.h"

scheduler_item_t *rootFill = NULL;
scheduler_item_t *rootEmpty = NULL;
scheduler_item_t *sch_storage_head = NULL;

volatile int scheduler_list_busy = 0;

static inline void spinlock(volatile int *lock)
{
    while(!__sync_bool_compare_and_swap(lock, 0, 1))
    {
        sched_yield();
    }
}
static inline void spinunlock(volatile int *lock)
{
    *lock = 0;
}

void release_scheduler() {
	if (sch_storage_head) free(sch_storage_head);
	rootEmpty = rootFill = sch_storage_head = NULL;
	
}

void init_scheduler() {
	int i;
	scheduler_item_t *tmp;

	release_scheduler();
	
	tmp = rootEmpty = sch_storage_head = (scheduler_item_t *) malloc( sizeof(scheduler_item_t) * MAX_SCHTASKS );
	for (i=0; i < MAX_SCHTASKS-1; i++) {
		tmp->next = tmp+1;
		tmp = tmp->next;
	}
	(tmp + MAX_SCHTASKS-1)->next = NULL;
}


int scheduler_addtask( unsigned short ngate, unsigned short dmts, o22_wood_t *wood, int check_lock) {
	scheduler_item_t *tmp;

	if (check_lock) spinlock(&scheduler_list_busy);

	if ( rootEmpty == NULL ) {
		if (check_lock) spinunlock(&scheduler_list_busy);
		return SCH_EOL;
	}

	tmp = rootEmpty;
	rootEmpty = tmp->next;
	tmp->next = rootFill;
	rootFill = tmp;

	memcpy( (void *)&tmp->wood, (void *)wood, sizeof(o22_wood_t) );
	tmp->dmts = dmts;
	tmp->ngate = ngate;
	
	if (check_lock) spinunlock(&scheduler_list_busy);
	
	return SCH_OK;
}

int scheduler_droptask(scheduler_item_t *t, int check_lock){
	scheduler_item_t *tmp;

	if (check_lock) spinlock(&scheduler_list_busy);
	
	if (t == rootFill) {
		rootFill = t->next;
	} else {
		tmp = rootFill;
		while (tmp) {
			if ( tmp->next == t) break;
			tmp++;
		} 
		if (tmp == NULL){
			if (check_lock) spinunlock(&scheduler_list_busy);
			return SCH_BADPTR;
		}
		tmp->next = t->next;
	}
	t->next = rootEmpty;
	rootEmpty = t;
	
	if (check_lock) spinunlock(&scheduler_list_busy);
	return SCH_OK;
}

void scheduler_dump() {
	scheduler_item_t *tmp;
	int i;

	spinlock(&scheduler_list_busy);
	for (tmp = rootEmpty,i = 0; !(tmp == NULL); tmp = tmp->next, i++) INFO_PRINTLC(i+1, 10, "%-d %x", i, tmp);
	printf ("\nSCH Empty: %d nodes\n", i);
	for (tmp = rootFill,i = 0; !(tmp == NULL); tmp = tmp->next, i++) {
		printf("SCH %x %d index: %d D1: %d D3: %d Length: %d\n", tmp, i, tmp->wood.index, tmp->wood.d1, tmp->wood.d3, tmp->wood.length);
	}
	spinunlock(&scheduler_list_busy);
}

scheduler_item_t * scheduler_findtask_by_wood_index(unsigned short index) {
	scheduler_item_t *tmp;
	spinlock(&scheduler_list_busy);
	tmp = rootFill;
	while (tmp) {
		if (tmp->wood.index == index) break;
		tmp = tmp->next;
	}
	spinunlock(&scheduler_list_busy);
	return tmp;
}

// ---------------- thread for check the wood's list
//#define timer_expired(t) \
//	((clock_time_t)(clock_time() - (t)->start) >= (clock_time_t)(t)->interval)


void * scheduler_executor(void *arg) {
	scheduler_item_t *tmp, *next;
	int i,k;
	unsigned short old_dmts;
	
	old_dmts = current_dmts;
	k = 0;
	
	while(1) {
		++k;

#ifdef INFO
		k %= 250;
		if (k == 0) {
			spinlock(&scheduler_list_busy);
			for (tmp = rootFill, i = 0 ; !(tmp==NULL); i++) {
				INFO_PRINTLC(i+1,25,"%d %c %x index:%d D1:%d D2:%d D3:%d Length:%d GATE:%d DMTS: %d", i, (tmp->dmts <= current_dmts)?'X':' ' , tmp,
					tmp->wood.index, tmp->wood.d1, tmp->wood.d2, tmp->wood.d3, tmp->wood.length,
					tmp->ngate, tmp->dmts);
			}
			spinunlock( &scheduler_list_busy );
		}
#endif

		if ( !(old_dmts == current_dmts) ) {
			spinlock(&scheduler_list_busy);
			for (tmp = rootFill; !(tmp==NULL);) {
				if ( tmp->dmts <= current_dmts ) {
					next = tmp->next;
					gate_run( tmp->ngate );
					scheduler_droptask(tmp, SCH_NOLOCK);
					tmp = next;
				} else {
					tmp = tmp->next;
				}
			}
			spinunlock( &scheduler_list_busy );
		}
		usleep(1000);
		old_dmts = current_dmts;
	}
}
