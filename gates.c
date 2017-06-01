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
	gate_t *rt;
	int i;
	
	for (i=0, rt = NULL; i<gates_num; i++) {
		if ( (wood->d2 < gates[i].dmin) || (gates[i].dmax < wood->d2) ) continue;
		rt = &gates[i];
	}
	return rt;
}
