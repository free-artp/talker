#include <stdlib.h>
#include <stdio.h>
#include <syslog.h>
#include <getopt.h>
#include <termios.h>

#include "comm.h"
#include "gates.h"

#include <iniparser.h>
#include <dictionary.h>


/*

dmts = 20
dmin = 0
dmax = 200
pindir = 1
dir = 1
delay = 100

*/
 
int config_gates(dictionary *ini, int gate_num){
	char buf[32];
	int n;
	
	sprintf(buf,"gate-%-d:dmts", gate_num+1);
	if( (gates[gate_num].dmts = iniparser_getint( ini, buf, -1 )) < 0 ) {
		syslog(LOG_ERR, "gate:%d no dmts", gate_num);
		return 1;
	}
	sprintf(buf,"gate-%-d:dmin", gate_num+1);
	if( (gates[gate_num].dmin = iniparser_getint( ini, buf, -1 )) < 0 ) {
		syslog(LOG_ERR, "gate:%d no dmin", gate_num);
		return 1;
	}
	sprintf(buf,"gate-%-d:dmax", gate_num+1);
	if( (gates[gate_num].dmax = iniparser_getint( ini, buf, -1 )) < 0 ) {
		syslog(LOG_ERR, "gate:%d no dmax", gate_num);
		return 1;
	}
	sprintf(buf,"gate-%-d:pin", gate_num+1);
	if( (gates[gate_num].pin = iniparser_getint( ini, buf, -1 )) < 0 ) {
		syslog(LOG_ERR, "gate:%d no pin", gate_num);
		return 1;
	}
	sprintf(buf,"gate-%-d:pindir", gate_num+1);
	if( (gates[gate_num].pindir = iniparser_getint( ini, buf, -1 )) < 0 ) {
		syslog(LOG_ERR, "gate:%d no pindir", gate_num);
		return 1;
	}
	sprintf(buf,"gate-%-d:dir", gate_num+1);
	if( (gates[gate_num].dir = iniparser_getint( ini, buf, -1 )) < 0 ) {
		syslog(LOG_ERR, "gate:%d no dir", gate_num);
		return 1;
	}
	sprintf(buf,"gate-%-d:delay", gate_num+1);
	if( (gates[gate_num].delay = iniparser_getint( ini, buf, -1 )) < 0 ) {
		syslog(LOG_ERR, "gate:%d no delay", gate_num);
		return 1;
	}

	
	return 0;
}

int config(int argc, char* argv[]) {
	int opt;
	int n;
	char buf[8];
	char *p, *conf_name;

	dictionary * ini;
	
	/* setup defaults */
	baud = DEFAULT_BAUD;
	pin_dtr = DEFAULT_DTR_PIN;
	send_delay = DEFAULT_SEND_DELAY;
	snprintf(fn_port, sizeof(fn_port), "%s", DEFAULT_SERIAL_PORT);
	main_delay = DEFAULT_DELAY;
	
	fd_port = -1;
	
	for(opt=getopt(argc, argv, "c:p:b:d:h"); opt != -1; opt=getopt(argc, argv, "m:p:b:d:")) {
		switch (opt) {
			case 'c' : { /* config file */
				ini = iniparser_load(optarg);
				if (ini == NULL){
					syslog(LOG_ERR, "couldn't open config file %s", optarg);
					exit(0);
				}
				syslog(LOG_INFO, "config: %s", optarg);
				if ( p = (char *) iniparser_getstring(ini, "Port:port", NULL) ) {
					strncpy(fn_port, p, strlen(p));
					syslog(LOG_INFO, "port: %s", fn_port);
				}
				if ( p = (char *) iniparser_getstring(ini, "Port:speed", NULL) ) {
					baud = set_baud( p );
					if (baud == 0) {
						syslog(LOG_ERR, "baud rate [%s] not supported", buf);
						closelog();
						exit(1);
					}
					syslog(LOG_INFO, "baudrate: %s 0x%x", p, baud);
				}
				if ( p = (char *) iniparser_getstring(ini, "Port:pin_dtr", NULL) ) {
					pin_dtr = atoi( p );
					if ( (pin_dtr<=0) || (pin_dtr> MAX_PIN)) {
						syslog(LOG_ERR, "DTR pin number [%s] not supported", p);
						closelog();
						exit(1);
					}
					syslog(LOG_INFO, "DTR pin: %d", pin_dtr);
				}
				if ( p = (char *) iniparser_getstring(ini, "Port:send_delay", NULL) ) {
					send_delay = atoi( p );
					if ( (send_delay<=0) || (send_delay> MAX_DELAY)) {
						syslog(LOG_ERR, "send delay [%s] not supported", p);
						closelog();
						exit(1);
					}
					syslog(LOG_INFO, "send delay: %d", send_delay);
				}
				
				if ( gates_num = iniparser_getint(ini, "Gates:number", 0) ) {
					gates = malloc( sizeof(gate_t) * gates_num);
					for (n=0; n < gates_num; n++ ){
						if ( config_gates(ini, n) ) {
							syslog(LOG_ERR, "could not configure gate %d", n);
							closelog();
							exit(1);
						}
						syslog(LOG_INFO, "gate-%-d dmts:%d dmin:%d dmax:%d pindir:%d dir:%d delay:%d",n,
							gates[n].dmts, gates[n].dmin, gates[n].dmax, gates[n].pindir, gates[n].dir, gates[n].delay);
					}
				}
				
				iniparser_freedict(ini);
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
			  main_delay = atoi(buf);
			  if (main_delay == 0) {
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
				printf("\t-d XXX - Delay to send in seconds. Default - %d\n", DEFAULT_DELAY);
				printf("\t-p <portname> - Serial port name. Default %s\n", DEFAULT_SERIAL_PORT);
				printf("\t-h - this text\n");
				closelog();
				exit(1);
			}; break;
		}; /* end switch */
	} /* end for */
	return 0;
}
