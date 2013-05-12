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


#ifndef SERIAL_H_
#define SERIAL_H_

typedef enum serial_baud {
	SERIAL_BAUD_1200,
	SERIAL_BAUD_1800,
	SERIAL_BAUD_2400,
	SERIAL_BAUD_4800,
	SERIAL_BAUD_9600,
	SERIAL_BAUD_19200,
	SERIAL_BAUD_38400,
	SERIAL_BAUD_57600,
	SERIAL_BAUD_115200
} serial_baud_t ;

typedef enum serial_bits {
	SERIAL_BITS_5,
	SERIAL_BITS_6,
	SERIAL_BITS_7,
	SERIAL_BITS_8
} serial_bits_t ;

typedef enum serial_parity {
	SERIAL_PARITY_NONE,
	SERIAL_PARITY_EVEN,
	SERIAL_PARITY_ODD
} serial_parity_t  ;

typedef enum serial_stop_bits {
	SERIAL_STOP_BITS_1,
	SERIAL_STOP_BITS_2
} serial_stop_bits_t ;

typedef enum serial_signals {
	SERIAL_SIGNAL_DTR,
	SERIAL_SIGNAL_RTS,
} serial_signals_t ;

typedef enum serial_errors {
	SERIAL_ERR_OK,
	SERIAL_ERR_INVALIG_ARGUMENT,
	SERIAL_ERR_SYSTEM
} serial_errors_t ;


int serial_open(const char *device);
serial_errors_t serial_flush(int fd);
serial_errors_t serial_close(int fd);
serial_errors_t serial_setup(int fd, serial_baud_t baud, serial_bits_t bits, serial_parity_t parity, serial_stop_bits_t stop_bits);
serial_errors_t serial_read(int fd, const void *buffer, int len);
serial_errors_t serial_write(int fd, const void *buffer, int len);
serial_errors_t serial_signal(int fd, serial_signals_t signal, int status);

#endif /* SERIAL_H_ */
