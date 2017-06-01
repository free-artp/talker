
#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <syslog.h>
#include <pthread.h>
#include <string.h>

#include "config.h"
#include "comm.h"
#include "scheduler.h"
#include "info.h"

#ifdef INFO
FILE * infoc;
#endif

int main(int argc, char* argv[]) {
	
	pthread_t twriter, treader, tscheduler;
	int result;
	
	openlog("talker", LOG_PID, LOG_USER);
	syslog(LOG_INFO, "start");

	if (config(argc, argv)) {
		perror("config troubles");
		exit(1);
	}

//	syslog(LOG_INFO, "port: %s speed: %0x", fn_port, baud);

	init_port();

	INFO_INIT();
	
	init_hard();
	init_scheduler();

	//----------------------------------------------------------
	result = pthread_create(&twriter, NULL, writer, NULL);
	if ( result != 0 ) {
		syslog(LOG_ERR, "could not create thread writer");
		exit(0);
	}
	syslog(LOG_INFO, "writer created");
	
	result = pthread_create(&treader, NULL, reader, NULL);
	if ( result != 0 ) {
		syslog(LOG_ERR, "could not create thread reader");
		exit(0);
	}
	syslog(LOG_INFO, "reader created");

	result = pthread_create(&tscheduler, NULL, scheduler_executor, NULL);
	if ( result != 0 ) {
		syslog(LOG_ERR, "could not create thread sheduler_executor");
		exit(0);
	}
	syslog(LOG_INFO, "scheduler_executor created");
	//----------------------------------------------------------
	result = pthread_join(twriter, NULL);
	if (result != 0) {
		perror("Joining the 1 thread");
		return EXIT_FAILURE;
	}
	result = pthread_join(treader, NULL);
	if (result != 0) {
		perror("Joining the 2 thread");
		return EXIT_FAILURE;
	}
	result = pthread_join(tscheduler, NULL);
	if (result != 0) {
		perror("Joining the 3 thread");
		return EXIT_FAILURE;
	}

//	writer((void*)0);
	printf("Done\n");
	exit(0);
	
  
}
