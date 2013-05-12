/*

   stm32loader Open Source flash loader program for ST STM32 microcontrollers.
   <konovalchukov.yakov@gmail.com>

   Copyright 2013 Yakov Konovalchukov

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <stdint.h>
#include <sys/ioctl.h>

#include "serial.h"

void hex_trace(const uint8_t *buffer, int len){
	int i = 0;
	for (i = 0; i < len; i++) {
		if (buffer[i] < 0x10)
			printf("0");
		printf("%x ", buffer[i] & (~((~0)<<8)));
	}
	printf("\n");
}

int serial_open(const char *device){

	int result = open(device, O_RDWR | O_NOCTTY | O_NDELAY);

	if (result > -1)
		fcntl(result, F_SETFL, 0);

	return result;
}

serial_errors_t serial_flush(int fd){

	if(fd < 0)
		return SERIAL_ERR_INVALIG_ARGUMENT;

	if (tcflush(fd, TCIFLUSH) != 0)
		return SERIAL_ERR_SYSTEM;

	return SERIAL_ERR_OK;
}

serial_errors_t serial_close(int fd){

	if(fd < 0)
		return SERIAL_ERR_INVALIG_ARGUMENT;

	close(fd);
}

serial_errors_t serial_read(int fd, const void *buffer, int len){

	int plen = len;

	if(fd < 0 || len < 0)
		return SERIAL_ERR_INVALIG_ARGUMENT;

	uint8_t *bufptr = (uint8_t*)buffer;
	int r = 0;

	while(len > 0){
		r = read(fd, bufptr, len);
		if (r < 1){
			return SERIAL_ERR_SYSTEM;
		}
		len -= r;
		bufptr += r;
	}

	printf("[%d] << ", plen);
	hex_trace(buffer, plen);

	return SERIAL_ERR_OK;
}

serial_errors_t serial_write(int fd, const void *buffer, int len){

	int plen = len;

	if(fd < 0 || len < 0)
		return SERIAL_ERR_INVALIG_ARGUMENT;

	uint8_t *bufptr = (uint8_t*)buffer;
	int r = 0;

	while(len > 0){
		r = write(fd, bufptr, len);
		if (r < 0)
			return SERIAL_ERR_SYSTEM;
		len -= r;
		bufptr += r;
	}

	printf("\n[%d] >> ", plen);
	hex_trace(buffer, plen);

	return SERIAL_ERR_OK;
}

serial_errors_t serial_setup(int fd, serial_baud_t baud, serial_bits_t bits, serial_parity_t parity, serial_stop_bits_t stop_bits){

	if(fd < 0)
		return SERIAL_ERR_INVALIG_ARGUMENT;

	speed_t		c_port_baud;
	tcflag_t	c_port_bits;
	tcflag_t	c_port_parity;
	tcflag_t	c_port_stop;

	tcflag_t	i_port_parity;

	switch(baud) {
		case SERIAL_BAUD_1200  : c_port_baud = B1200  ; break;
		case SERIAL_BAUD_1800  : c_port_baud = B1800  ; break;
		case SERIAL_BAUD_2400  : c_port_baud = B2400  ; break;
		case SERIAL_BAUD_4800  : c_port_baud = B4800  ; break;
		case SERIAL_BAUD_9600  : c_port_baud = B9600  ; break;
		case SERIAL_BAUD_19200 : c_port_baud = B19200 ; break;
		case SERIAL_BAUD_38400 : c_port_baud = B38400 ; break;
		case SERIAL_BAUD_57600 : c_port_baud = B57600 ; break;
		case SERIAL_BAUD_115200: c_port_baud = B115200; break;
		default:
			return SERIAL_ERR_INVALIG_ARGUMENT;
	}

	switch(bits) {
		case SERIAL_BITS_5: c_port_bits = CS5; break;
		case SERIAL_BITS_6: c_port_bits = CS6; break;
		case SERIAL_BITS_7: c_port_bits = CS7; break;
		case SERIAL_BITS_8: c_port_bits = CS8; break;

		default:
			return SERIAL_ERR_INVALIG_ARGUMENT;
	}

	switch(parity) {
		case SERIAL_PARITY_NONE: c_port_parity = 0; 				i_port_parity = 0; 		break;
		case SERIAL_PARITY_EVEN: c_port_parity = PARENB; 			i_port_parity = INPCK;	break;
		case SERIAL_PARITY_ODD : c_port_parity = PARENB | PARODD; 	i_port_parity = INPCK;	break;

		default:
			return SERIAL_ERR_INVALIG_ARGUMENT;
	}

	switch(stop_bits) {
		case SERIAL_STOP_BITS_1: c_port_stop = 0;	   break;
		case SERIAL_STOP_BITS_2: c_port_stop = CSTOPB; break;

		default:
			return SERIAL_ERR_INVALIG_ARGUMENT;
	}

	switch(stop_bits) {
		case SERIAL_STOP_BITS_1: c_port_stop = 0;	   break;
		case SERIAL_STOP_BITS_2: c_port_stop = CSTOPB; break;

		default:
			return SERIAL_ERR_INVALIG_ARGUMENT;
	}

	struct termios settings;

	tcgetattr(fd, &settings);

	///////////////////////////////////
	// Control options
	///////////////////////////////////

	// Enable the receiver and set local mode.
	settings.c_cflag |= (CLOCAL | CREAD);

	// Set baud rate.
	settings.c_cflag &= ~CBAUD;
	cfsetispeed(&settings, c_port_baud);
	cfsetospeed(&settings, c_port_baud);

	// Set character size.
	settings.c_cflag &= ~CSIZE;
	settings.c_cflag |= c_port_bits;

	// Set parity settings
	settings.c_cflag &= ~(PARENB | PARODD);
	settings.c_cflag |= c_port_parity;

	// Set stop bit
	settings.c_cflag &= ~CSTOPB;
	settings.c_cflag |= c_port_stop;

	// Disable hardware flow control
	settings.c_cflag &= ~CRTSCTS;

	// Drop DTR on close
	settings.c_cflag |= HUPCL;

	///////////////////////////////////
	// Local options
	///////////////////////////////////

	// Choosing raw input.
	settings.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

	///////////////////////////////////
	// Input options
	///////////////////////////////////

	// Disable strip parity bit.
	settings.c_iflag &= ~(INPCK | ISTRIP);
	settings.c_iflag |= i_port_parity;

	// Disable software flow control.
	settings.c_iflag &= ~(IXON | IXOFF | IXANY);

	///////////////////////////////////
	// Output options
	///////////////////////////////////

	// Choosing raw output.
	settings.c_oflag &= ~OPOST;

	///////////////////////////////////
	// Control characters
	///////////////////////////////////

	settings.c_cc[VMIN ] = 0;
	settings.c_cc[VTIME] = 30;

	if (tcsetattr(fd, TCSANOW, &settings) != 0)
		return SERIAL_ERR_SYSTEM;

	return SERIAL_ERR_OK;
}

serial_errors_t serial_signal(int fd, serial_signals_t signal, int value){

	if(fd < 0)
		return SERIAL_ERR_INVALIG_ARGUMENT;

	int signal_flag = 0;

	switch(signal){
		case SERIAL_SIGNAL_DTR: signal_flag = TIOCM_DTR; break;
		case SERIAL_SIGNAL_RTS: signal_flag = TIOCM_RTS; break;
		default:
			return SERIAL_ERR_INVALIG_ARGUMENT;
	}


	int status;

	ioctl(fd, TIOCMGET, &status);

	if (value)
		status |= signal_flag;
	else
		status &= ~signal_flag;

	if(ioctl(fd, TIOCMSET, &status) == -1)
		return SERIAL_ERR_SYSTEM;

	return SERIAL_ERR_OK;
}

