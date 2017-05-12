

#ifndef COMM_H
#define COMM_H


#define DEFAULT_SERIAL_PORT "/dev/ttyS0"
#define DEFAULT_BAUD B57600
#define DEFAULT_MESSAGE "test"
#define DEFAULT_DELAY 1

char	buffer[2048];
char	fn_port[128];
int		fd_port;

int		baud;
int		delay;

char	message[10];

int set_baud(char* baud_s);
int set_interface_attribs (int fd, int speed, int parity);
void set_blocking (int fd, int should_block);


void * writer(void *arg);
void * reader(void *arg);

#endif
