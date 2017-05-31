#include <syslog.h>
#include <stdlib.h>

#include "opto22.h"
#include "gates.h"
#include "scheduler.h"

#include <stdio.h>
#include "info.h"

gate_t *gates;
int gates_num;

void gate_run( int gate_num) {
#ifdef DEBUG
	syslog(LOG_INFO, "--------gate %d run", gate_num);
#endif
}

gate_t * gates_choose_by_wood( o22_wood_t *wood) {
	gate_t *gt, *rt;
	int i;
	
	for (i=0, rt = NULL, gt = gates; i<gates_num; i++, gt++) {
		if ( (wood->d2 < gt->dmin) || (gt->dmax < wood->d2) ) continue;
		rt = gt;
	}
	return rt;
}
/*
void gate_check(unsigned short dmts) {
	scheduler_item_t *t1, *t2;
	int i;
	
	i=0;
	INFO_PRINTLC(22,10,"DMTS:%d", dmts);
	
	t1 = rootFill;
	while(  !( t1 == NULL ) ) {
		t2 = t1;
		
		INFO_PRINTLC(23+i,10,"%d %d %d", t1->wood.index, t1->ngate, t1->dmts);
		
		if ( dmts >= t2->dmts ) {
			INFO_PRINTLC(23+i,40,"RUN %d", 1);
			gate_run(t2->ngate);
			t1 = t2->next;
			scheduler_droptask(t2);
		} else {
			t1 = t1->next;
		}
		i++;
	}
}

*/
