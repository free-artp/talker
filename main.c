
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

int main(int argc, char* argv[]) {
	
	pthread_t twriter, treader;
	int result;
	
	
	openlog("talker", LOG_PID, LOG_USER);
	syslog(LOG_INFO, "start");

	if (config(argc, argv)) {
		perror("config troubles");
		exit(1);
	}

	syslog(LOG_INFO, "port: %s speed: %0x", fn_port, baud);

	fd_port = open (fn_port, O_RDWR | O_NOCTTY | O_NDELAY);
	
	if (fd_port < 0)
	{
			syslog(LOG_ERR, "error %d opening %s: %s", errno, fn_port, strerror (errno) );
			closelog();
			exit(1);
	}

	syslog(LOG_INFO, "port %s opened, fd:%d", fn_port, fd_port);

	set_interface_attribs (fd_port, baud, 0);  // set speed to 115,200 bps, 8n1 (no parity)
	set_blocking (fd_port, 0);                // set no blocking

	syslog(LOG_INFO, "port configured");

	init_hard();
	

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
	//----------------------------------------------------------
	result = pthread_join(twriter, NULL);
	if (result != 0) {
		perror("Joining the first thread");
		return EXIT_FAILURE;
	}
	result = pthread_join(treader, NULL);
	if (result != 0) {
		perror("Joining the second thread");
		return EXIT_FAILURE;
	}

//	writer((void*)0);
	printf("Done\n");
	exit(0);
	
  
}
