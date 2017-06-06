#include <stdlib.h>
#include <stdio.h>
#include <syslog.h>
#include <getopt.h>
#include <termios.h>

#include "comm.h"
#include "gates.h"
#include "scheduler.h"

#include <iniparser.h>
#include <dictionary.h>

char progname[128];
char pid_file[128];
char *config_filename = NULL;
int mode_daemon = 0;

/*

dmts = 20
dmin = 0
dmax = 200
pindir = 1
dir = 1
delay = 100

*/

// return 0 - success
// return 1 - fail
 
int config_gate(dictionary *ini, int gate_num){
	char buf[32];
	
	gates[gate_num].num = gate_num;
	sprintf(buf,"gate-%-d:dmts", gate_num+1);
	if( (gates[gate_num].dmts_delay = iniparser_getint( ini, buf, -1 )) < 0 ) {
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
// return	0 - success
//			1 - wrong params
//			2 - not open fail
int load_config_file(char* conf_name) {
	dictionary * ini;
	char *p;
	int n;
	
	if (conf_name && (config_filename==NULL)) {
		config_filename = strdup(conf_name);
	}
	
	ini = iniparser_load(config_filename);
	
	if (ini == NULL){
		syslog(LOG_ERR, "couldn't open config file %s", conf_name);
		return 2;
	}
	syslog(LOG_INFO, "load config from file: %s", conf_name);
	// Main section
	if ( p = (char *) iniparser_getstring(ini, "Main:main_delay", NULL) ) {
		main_delay = atoi( p );
		if ( (main_delay<=0) || (main_delay> MAX_DELAY)) {
			syslog(LOG_ERR, "send delay [%s] not supported", p);
			return 1;
		}
		syslog(LOG_INFO, "send delay: %d", main_delay);
	}
	// Port section
	if ( p = (char *) iniparser_getstring(ini, "Port:port", NULL) ) {
		strncpy(fn_port, p, strlen(p));
		syslog(LOG_INFO, "port: %s", fn_port);
	}
	if ( p = (char *) iniparser_getstring(ini, "Port:speed", NULL) ) {
		baud = set_baud( p );
		if (baud == 0) {
			syslog(LOG_ERR, "baud rate [%s] not supported", p);
			return 1;
		}
		syslog(LOG_INFO, "baudrate: %s 0x%x", p, baud);
	}
	if ( p = (char *) iniparser_getstring(ini, "Port:pin_dtr", NULL) ) {
		pin_dtr = atoi( p );
		if ( (pin_dtr<=0) || (pin_dtr> MAX_PIN)) {
			syslog(LOG_ERR, "DTR pin number [%s] not supported", p);
			return 1;
		}
		syslog(LOG_INFO, "DTR pin: %d", pin_dtr);
	}
	if ( p = (char *) iniparser_getstring(ini, "Port:send_delay", NULL) ) {
		send_delay = atoi( p );
		if ( (send_delay<=0) || (send_delay> MAX_DELAY)) {
			syslog(LOG_ERR, "send delay [%s] not supported", p);
			return 1;
		}
		syslog(LOG_INFO, "send delay: %d", send_delay);
	}
	// Gates section
	if ( gates_num = iniparser_getint(ini, "Gates:number", 0) ) {
		gates = malloc( sizeof(gate_t) * gates_num);
		for (n=0; n < gates_num; n++ ){
			if ( config_gate(ini, n) ) {
				syslog(LOG_ERR, "could not configure gate %d", n);
				return 1;
			}
			syslog(LOG_INFO, "gate-%-d dmts:%d dmin:%d dmax:%d pindir:%d dir:%d delay:%d",n,
				gates[n].dmts_delay, gates[n].dmin, gates[n].dmax, gates[n].pindir, gates[n].dir, gates[n].delay);
		}
	}

	iniparser_freedict(ini);

	return 0;
}
// return	1 - fail
//			0 - success
int config(int argc, char* argv[]) {
	int opt;
	int n;
	char buf[8];
	
	/* setup defaults */
//	if (progname == NULL){
//	}
	sprintf( progname, "%s",  basename(argv[0]));

	memset(pid_file, 0, sizeof(pid_file));
	sprintf(pid_file, "/var/run/%s.pid", progname);

	openlog( progname, LOG_PID, LOG_USER);
	syslog(LOG_INFO, "starting");

	baud = DEFAULT_BAUD;
	pin_dtr = DEFAULT_DTR_PIN;
	send_delay = DEFAULT_SEND_DELAY;
	snprintf(fn_port, sizeof(fn_port), "%s", DEFAULT_SERIAL_PORT);
	main_delay = DEFAULT_DELAY;
	
	fd_port = -1;
	
	for(opt=getopt(argc, argv, "c:p:b:m:dh"); opt != -1; opt=getopt(argc, argv, "c:p:b:m:dh")) {
		switch (opt) {
			case 'c' : { /* config file */
				if ( n= load_config_file(optarg) ) {
					if (n == 2)	syslog(LOG_ERR, "couldn't open config %s", optarg);
					else syslog(LOG_ERR, "wrong config %s", optarg);
					return 1;
				}
			}; break;
			case 'b' : { /* set baud rate */
			  n = snprintf(buf, sizeof(buf), "%s", optarg);
			  if (n < 0 || n > sizeof(buf)) {
				syslog(LOG_ERR, "couldn't convert -b option into a baud rate");
				return 1;
			  }
			  baud = set_baud(buf);
			  if (baud == 0) {
				syslog(LOG_ERR, "baud rate [%s] not supported", buf);
				return 1;
			  }
			}; break;
			case 'm' : { /* set delay */
			  n = snprintf(buf, sizeof(buf), "%s", optarg);
			  if (n < 0 || n > sizeof(buf)) {
				syslog(LOG_ERR, "couldn't convert -t option into a delay");
				return 1;
			  }
			  main_delay = atoi(buf);
			  if (main_delay == 0) {
				syslog(LOG_ERR, "delay [%s] not supported", buf);
				return 1;
			  }
			}; break;
			case 'p' : { /* serial device */
			  n = snprintf(fn_port, sizeof(fn_port), "%s", optarg);
			  if (n < 0 || n > sizeof(fn_port)) {
				syslog(LOG_ERR, "output filename truncated, longer than %d bytes", sizeof(fn_port));
				return 1;
			  }
			}; break;
#ifdef DAEMON
			case 'd' : { /* daemon_mode */
				mode_daemon = 1;
			}; break;
#endif
			default:
			case 'h' : { /* usage */
				printf("usage %s [options]\nWhere options are:\n", argv[0]);
				printf("\t-b XXXX - baud rate. XXXX-300, 1200 ... 115200. Default -57600\n");
				printf("\t-m XXX - Main delay to send in seconds. Default - %d\n", DEFAULT_DELAY);
				printf("\t-p <portname> - Serial port name. Default %s\n", DEFAULT_SERIAL_PORT);
#ifdef DAEMON
				printf("\t-d - daemoize\n");
#endif
				printf("\t-h - this text\n");
				closelog();
				exit(1);
			}; break;
		}; /* end switch */
	} /* end for */
	return 0;
}

// функция которая загрузит конфиг заново
// и внесет нужные поправки в работу
int reload_config()
{
	spinlock( &scheduler_list_busy );
	spinlock( &port_busy_w );
	spinlock( &port_busy_r );
	
	release_scheduler;
	free(gates);
	
	if (load_config_file(config_filename)) {
		syslog(LOG_ERR,"could not reload config %s", config_filename);
		return 0;
	}
	
	init_scheduler();
	init_port();
	
	spinunlock( &port_busy_w );
	spinunlock( &port_busy_r );
	spinunlock( &scheduler_list_busy );
	
	return 1;
}


