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

#include <arpa/inet.h>

#include <wiringPi.h>

#include "comm.h"
#include "opto22.h"
#include "scheduler.h"
#include "gates.h"
#include "info.h"

//----------------------------------------

unsigned char	buffer[BUFF_LEN];
unsigned int buff_cnt;

int	fl_answer=0;
volatile unsigned short current_dmts=0;


#ifdef DEBUG
unsigned char	s_buffer[BUFF_LEN];
#endif

char	fn_port[128];		// имя порта
int		fd_port;			// дескриптор порта
int		baud;				// скорость порта
int		pin_dtr;			// пин для переключения прием.передатчика
int		send_delay;			// задержка до выключения приемника

//----------------------------------------

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

		cfmakeraw(&tty);

        tty.c_cflag &= ~CSTOPB;
        tty.c_cflag &= ~CRTSCTS;
        tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
        tty.c_cc[VMIN]  = 0;            // read doesn't block
        tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout


/*
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
*/
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

void init_hard() {
	wiringPiSetup();
	pinMode ( pin_dtr, OUTPUT);
	
	fl_answer = 0;
	buff_cnt = 0;
}

//================================================= crc calculation

void tohex(unsigned char * in, size_t insz, char * out, size_t outsz)
{
    unsigned char * pin = in;
    const char * hex = "0123456789ABCDEF";
    char * pout = out;
    for(; pin < in+insz; pout +=2, pin++){
        pout[0] = hex[(*pin>>4) & 0xF];
        pout[1] = hex[ *pin     & 0xF];
        if (pout + 2 - out > outsz){
            /* Better to truncate output string than overflow buffer */
            /* it would be still better to either return a status */
            /* or ensure the target buffer is large enough and it never happen */
            break;
        }
    }
    *pout = 0;
}


unsigned short crc16(const unsigned char* data_p, unsigned char length){
    unsigned char x,i;
    unsigned short crc = 0;		// CRC-16 XMODEM. Otherwise 0xFFFF

	i = length;
    while (i--){
        x = crc >> 8 ^ *data_p++;
        x ^= x>>4;
        crc = (crc << 8) ^ ((unsigned short)(x << 12)) ^ ((unsigned short)(x <<5)) ^ ((unsigned short)x);
    }
    return crc;
}


//=================================================
//
// 110 		- 01-04-6e-04-c2d5
// 103(514) - 01 06 67 04 02 02 1a3a


int mk_req( unsigned char cmd, unsigned char* dop, size_t ldop) {
	o22_header_t* header;
	unsigned char* buffp;
	
	header = (o22_header_t*)buffer;

	bzero(header, sizeof(header)+ldop+2 );
	header->address = 0x01;
	header->cmd = cmd;
	header->dop_byte_num = 4;

	buffp = (unsigned char*)header + header->dop_byte_num;

	switch (cmd) {
		case CMD_SCANER_INFO:
			header->packet_size = 4;
			break;
		case CMD_SET_DATEIME:
			header->packet_size = 8;
			break;
		case CMD_COMMON_INFO:
			header->packet_size = 4;
			break;
		case CMD_WOOD_INFO:
			header->packet_size = 6;
			break;
	}
	if (ldop > 0) memcpy(buffp, dop, ldop);
	buffp += ldop;
	
//	*((unsigned short*)buffp) = ntohs( crc16( buffer, header->packet_size ) );
	*((unsigned short*)buffp) = htons( crc16( buffer, header->packet_size ) );
	return header->packet_size + 2;
}



//=================================================

void * writer(void *arg) {
	int l,i;
	unsigned char* s;
	o22_header_t* header;
	unsigned short crc;
	
	header = (o22_header_t*)buffer;
	
	while (1) {
		
		sleep( main_delay );
		
#ifdef DEBUG_SYSLOG
		if (buff_cnt) {
			crc = crc16(buffer, header->packet_size);
			tohex(buffer, buff_cnt, s_buffer, sizeof(s_buffer));
			syslog(LOG_INFO, "writer: [%d] %s - %04x", buff_cnt, s_buffer, crc);
		}
#endif

		memset(buffer,(unsigned char)0,sizeof(buffer));
		l = mk_req(CMD_COMMON_INFO, (void*)0, 0);

#ifdef DEBUG
		tohex(buffer, l, s_buffer, sizeof(s_buffer));
		syslog(LOG_INFO, "writer send %d bytes %s", l, s_buffer);
#endif
		
		digitalWrite (pin_dtr, HIGH);
		
		fl_answer = 1;
		buff_cnt = 0;
		
		write(fd_port, buffer, l);
		usleep(send_delay);  // 19200: min 3000, max 4000  = 3500! А в питоне - 12 ms
		
		digitalWrite (pin_dtr, LOW);

	}
}

//=================================================

void * reader(void *arg) {
	struct pollfd  poll_fd;
	int n;
	unsigned short* ind;
	o22_header_t* header;
	o22_common_t* cmn;
	unsigned short crc, crc_buff;
	gate_t *gt;
	
	poll_fd.fd = fd_port;
	poll_fd.events = POLLIN;
	poll_fd.revents = 0;

	header = (o22_header_t*)buffer;

	while (1) {
		n = poll(&poll_fd, 1, -1); // wait here
		
		if (n < 0) {
			syslog(LOG_ERR, "reader error %d %s", errno, strerror (errno) );
			break;
		}

		if (poll_fd.revents & POLLIN) {
			n = read(poll_fd.fd, &buffer[buff_cnt], sizeof(buffer)-buff_cnt );
			buff_cnt += n;
			poll_fd.revents = 0;
		
			if ( fl_answer > 0 && buff_cnt>= header->packet_size+2 ) {
				fl_answer = 0;
				INFO_CLS();
#ifdef DEBUG_SYSLOG				
				syslog(LOG_INFO, "reader header: buff_cnt: %d size:%d dn:%d",
					buff_cnt,
					(int)header->packet_size,
					(int)header->dop_byte_num );
#endif
				crc = crc16(buffer, header->packet_size);
				crc_buff = buffer[header->packet_size] <<8 | buffer[header->packet_size+1];
				if (crc != crc_buff) {
					syslog(LOG_ERR, "reader: crc error %04x %04x", crc, crc_buff);
					continue;
				}
				
				cmn  = (o22_common_t*)&buffer[4];
				current_dmts = cmn->cur_dmts;
				
#ifdef DEBUG_SYSLOG				
				syslog(LOG_INFO, "reader crc: %04x buff.end: %02x%02x index: %d d1: %d d2: %d d3: %d lowA:%d upA:%d",
					crc, buffer[header->packet_size],buffer[header->packet_size+1],
					cmn->last.index, cmn->last.d1, cmn->last.d3, cmn->last.length,
					cmn->cur_low_beamA, cmn->cur_upp_beamA);
#endif
				//printf("wood %d\n",cmn->last.index);
				INFO_PRINTLC(22,25,"WOOD:%d", cmn->last.index);

				if (scheduler_findtask_by_wood_index(cmn->last.index) == NULL) {
					syslog(LOG_INFO, "reader: new wood %d", cmn->last.index);
					if ( (gt = gates_choose_by_wood( &cmn->last )) == NULL ) {
						syslog(LOG_ERR, "gate could not find gate for wood %d", cmn->last.index);
						continue;
					} else {
						if (scheduler_addtask( gt->num, cmn->cur_dmts + gt->dmts, &cmn->last, SCH_LOCK ) < 0 ){
							syslog(LOG_ERR, "reader: no place for new data");
							continue;
						}
					}
				}
//				scheduler_dump();
//				gate_check(cmn->cur_dmts);
			}


			buff_cnt = (buff_cnt >= 2047)?0:buff_cnt;
			
		}
	}
}



