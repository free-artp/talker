#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <sys/types.h>
#include <errno.h>
#include <poll.h>
#include <fcntl.h>
#include <unistd.h>
#include <syslog.h>
#include <pthread.h>
#include <string.h>

#include "comm.h"

int set_baud(char* baud_s) {
  int b;
  int baud;

  b = atoi(baud_s);
  switch (b) {
  case 300 : baud = B300; break;
  case 1200 : baud = B1200; break;
  case 2400 : baud = B2400; break;
  case 9600 : baud = B9600; break;
  case 19200 : baud = B19200; break;
  case 38400 : baud = B38400; break;
  case 57600 : baud = B57600; break;
  case 115200 : baud = B115200; break;
  default : baud = 0;
  };
  return baud;
}



int set_interface_attribs (int fd, int speed, int parity)
{
        struct termios tty;
        memset (&tty, 0, sizeof tty);
        if (tcgetattr (fd, &tty) != 0)
        {
                syslog(LOG_ERR,"error %d from tcgetattr", errno);
                return -1;
        }

        cfsetospeed (&tty, speed);
        cfsetispeed (&tty, speed);

        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
        // disable IGNBRK for mismatched speed tests; otherwise receive break
        // as \000 chars
        tty.c_iflag &= ~IGNBRK;         // disable break processing
        tty.c_lflag = 0;                // no signaling chars, no echo,
                                        // no canonical processing
        tty.c_oflag = 0;                // no remapping, no delays
        tty.c_cc[VMIN]  = 0;            // read doesn't block
        tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

        tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

        tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
                                        // enable reading
        tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
        tty.c_cflag |= parity;
        tty.c_cflag &= ~CSTOPB;
        tty.c_cflag &= ~CRTSCTS;

        if (tcsetattr (fd, TCSANOW, &tty) != 0)
        {
                syslog(LOG_ERR,"error %d from tcsetattr", errno);
                return -1;
        }
        return 0;
}

void set_blocking (int fd, int should_block)
{
        struct termios tty;
        memset (&tty, 0, sizeof tty);
        if (tcgetattr (fd, &tty) != 0)
        {
                syslog(LOG_ERR,"error %d from tggetattr", errno);
                return;
        }

        tty.c_cc[VMIN]  = should_block ? 1 : 0;
        tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

        if (tcsetattr (fd, TCSANOW, &tty) != 0)
                syslog(LOG_ERR,"error %d setting term attributes", errno);
}



void * writer(void *arg) {

	while (1) {
		write(fd_port, message, strlen(message));
		syslog(LOG_INFO, "-------------------------writer: %s", message);
		sleep( delay );
	}
}

void * reader(void *arg) {
	struct pollfd  poll_fd;
	int n,i;

	poll_fd.fd = fd_port;
	poll_fd.events = POLLIN;
	poll_fd.revents = 0;


	while (1) {
		n = poll(&poll_fd, 1, -1); // wait here
		
		syslog(LOG_INFO, "reader: %d 0x%X", n, poll_fd.revents);
		
		if (n < 0) {
			break;
		}
		if (poll_fd.revents & POLLIN) {
			n = read(poll_fd.fd, buffer, sizeof(buffer));
			buffer[n] = 0;
			syslog(LOG_INFO, "reader: %d %s", n, buffer);
			poll_fd.revents = 0;
		}
	}
}


