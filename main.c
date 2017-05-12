
#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>
#include <syslog.h>
#include <pthread.h>
#include <string.h>

#include "comm.h"

int main(int argc, char* argv[]) {
	pthread_t twriter, treader;
	int result;
	int opt;
	char buf[8];
	int n;
	int err;
//	struct termios info;
	
	/* setup defaults */
	baud = DEFAULT_BAUD;
	snprintf(fn_port, sizeof(fn_port), "%s", DEFAULT_SERIAL_PORT);
	snprintf(message, sizeof(message), "%s", DEFAULT_MESSAGE);
	delay = DEFAULT_DELAY;
	
	fd_port = -1;
	
	openlog("talker", LOG_PID, LOG_USER);
	syslog(LOG_INFO, "start");

	for(opt=getopt(argc, argv, "m:p:b:d:h"); opt != -1; opt=getopt(argc, argv, "m:p:b:d:")) {
		switch (opt) {
			case 'm' : { /* set message */
				n = snprintf(message, sizeof(message), "%s", optarg);
				if (n < 0 || n > sizeof(buf)) {
					syslog(LOG_ERR, "mesage truncated, longer than %d bytes", sizeof(message));
					closelog();
					exit(1);
				};
			}; break;
			case 'b' : { /* set baud rate */
			  n = snprintf(buf, sizeof(buf), "%s", optarg);
			  if (n < 0 || n > sizeof(buf)) {
				syslog(LOG_ERR, "couldn't convert -b option into a baud rate");
				closelog();
				exit(1);
			  }
			  baud = set_baud(buf);
			  if (baud == 0) {
				syslog(LOG_ERR, "baud rate [%s] not supported", buf);
				closelog();
				exit(1);
			  }
			}; break;
			case 'd' : { /* set delay */
			  n = snprintf(buf, sizeof(buf), "%s", optarg);
			  if (n < 0 || n > sizeof(buf)) {
				syslog(LOG_ERR, "couldn't convert -t option into a delay");
				closelog();
				exit(1);
			  }
			  delay = atoi(buf);
			  if (delay == 0) {
				syslog(LOG_ERR, "baud rate [%s] not supported", buf);
				closelog();
				exit(1);
			  }
			}; break;
			case 'p' : { /* serial device */
			  n = snprintf(fn_port, sizeof(fn_port), "%s", optarg);
			  if (n < 0 || n > sizeof(fn_port)) {
				syslog(LOG_ERR, "output filename truncated, longer than %d bytes", sizeof(fn_port));
				closelog();
				exit(1);
			  }
			}; break;
			case 'h' : { /* usage */
				printf("usage %s [options]\nWhere options are:\n", argv[0]);
				printf("\t-b XXXX - baud rate. XXXX-300, 1200 ... 115200. Default -57600\n");
				printf("\t-m <message> - Message to send. Default - %s\n",DEFAULT_MESSAGE);
				printf("\t-d XXX - Delay to send in seconds. Default - %d\n", DEFAULT_DELAY);
				printf("\t-p <portname> - Serial port name. Default %s\n", DEFAULT_SERIAL_PORT);
				printf("\t-h - this text\n");
				closelog();
				exit(1);
			}; break;
		}; /* end switch */
	} /* end for */

	syslog(LOG_INFO, "port: %s speed: %0x", fn_port, baud);

//	fd_port = open (fn_port, O_RDWR | O_NOCTTY | O_SYNC);
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
	
	printf("Done\n");
	exit(0);
	
  
}
