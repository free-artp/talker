

#ifndef COMM_H
#define COMM_H


	#define DEFAULT_SERIAL_PORT "/dev/ttyS0"
	#define DEFAULT_BAUD B19200
	#define DEFAULT_MESSAGE "test"
	#define DEFAULT_DELAY 2
	#define DEFAULT_DTR_PIN 1
	#define DEFAULT_SEND_DELAY 3500

	#define MAX_PIN 20
	#define MAX_DELAY 1000000

	#define BUFF_LEN 2048

	extern unsigned char	buffer[BUFF_LEN];
	extern unsigned int		buff_cnt;

	#ifdef DEBUG
	extern unsigned char	s_buffer[BUFF_LEN];
	#endif

	extern char		fn_port[128];		// имя порта
	extern int		fd_port;			// дескриптор порта
	extern int		baud;				// скорость на порту
	extern int		pin_dtr;			// пин для переключения прием.передатчика
	extern int		send_delay;			// задержка до выключения приемника (usec)

	int		main_delay;

	int set_baud(char* baud_s);
	int set_interface_attribs (int fd, int speed, int parity);
	void set_blocking (int fd, int should_block);

	void init_hard();

	void * writer(void *arg);
	void * reader(void *arg);

#endif
