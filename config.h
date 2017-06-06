#ifndef _CONFIG_H
#define _CONFIG_H


int config(int argc, char* argv[]);		// return	0 - success, 1 - fail
int load_config_file(char* conf_name);	// return	0 - success, 1 - wrong params, 2 - not open fail
int reload_config();

extern char progname[128];

extern char pid_file[128];			// "/var/run/my_daemon.pid"
extern char *config_filename;

extern int mode_daemon;

#endif

