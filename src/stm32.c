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


#include <assert.h>
#include <stdint.h>
#include "stm32.h"
#include "serial.h"

#define STM32_INIT				(uint8_t)0x7F
#define STM32_ACK				(uint8_t)0x79
#define STM32_NACK				(uint8_t)0x1F
#define STM32_GET				(uint8_t)0x00
#define STM32_GET_RPS			(uint8_t)0x01
#define STM32_GET_ID			(uint8_t)0x02
#define STM32_READ				(uint8_t)0x11
#define STM32_WRITE				(uint8_t)0x31
#define STM32_EXTENDED_ERASE	(uint8_t)0x44
#define STM32_WRITE_PROTECT		(uint8_t)0x63
#define STM32_WRITE_UNPROTECT	(uint8_t)0x73
#define STM32_READOUT_PROTECT	(uint8_t)0x82
#define STM32_READOUT_UNPROTECT	(uint8_t)0x92

#define STM32_EE_ERASE_MASS		(uint16_t)0xFFFF
#define STM32_EE_ERASE_BANK1	(uint16_t)0xFFFE
#define STM32_EE_ERASE_BANK2	(uint16_t)0xFFFD

static stm32_errors_t send_cmd(int fd, uint8_t cmd, uint8_t *response){

	uint8_t buffer[2];

	buffer[0] = cmd;
	buffer[1] = buffer[0] ^ 0xFF;

	if (
		serial_write(fd, buffer, 2) != SERIAL_ERR_OK ||
		serial_read(fd, response, 1) != SERIAL_ERR_OK
	)
		return STM32_ERR_SERIAL;

	return STM32_ERR_OK;
}

stm32_errors_t stm32_init(int fd) {

	uint8_t buffer = STM32_INIT;

	/* send 'init' command and wait ACK */
	if (
		serial_write(fd, &buffer, 1) != SERIAL_ERR_OK ||
		serial_read(fd, &buffer, 1) != SERIAL_ERR_OK
	)
		return STM32_ERR_SERIAL;

	if (buffer != STM32_ACK)
		return STM32_ERR_PROTOCOL;

	return STM32_ERR_OK;
}

stm32_errors_t stm32_get(int fd, uint8_t *version, uint8_t **supported_commands, uint8_t *supported_commands_size){

	static uint8_t commands[0xFF];

	uint8_t buffer[0xFF];
	uint16_t len;

	/* send 'get' command and read response */
	stm32_errors_t result = send_cmd(fd, STM32_GET, buffer);

	if (result != STM32_ERR_OK)
		return result;

	if (buffer[0] != STM32_ACK)
		return STM32_ERR_PROTOCOL;

	/* read size of response */
	if(serial_read(fd, buffer, 1) != SERIAL_ERR_OK)
		return STM32_ERR_SERIAL;

	if (buffer[0] < 0)
		return STM32_ERR_PROTOCOL;

	len = buffer[0] + 1;

	/* read bootloader version */
	if(serial_read(fd, version, 1) != SERIAL_ERR_OK)
		return STM32_ERR_SERIAL;

	len--;

	/* read supported commands */
	if(serial_read(fd, commands, len) != SERIAL_ERR_OK)
		return STM32_ERR_SERIAL;

	*supported_commands = commands;
	*supported_commands_size = len;

	/* read ACK */
	if(serial_read(fd, buffer, 1) != SERIAL_ERR_OK)
		return STM32_ERR_SERIAL;

	if (buffer[0] != STM32_ACK)
		return STM32_ERR_PROTOCOL;

	return STM32_ERR_OK;
}

stm32_errors_t stm32_get_prs(int fd, uint8_t *rpdc, uint8_t *rpec){

	uint8_t buffer[0xFF];

	/* send 'get prs' command and read response */
	stm32_errors_t result = send_cmd(fd, STM32_GET_RPS, buffer);

	if (result != STM32_ERR_OK)
		return result;

	if (buffer[0] != STM32_ACK)
		return STM32_ERR_PROTOCOL;

	if (buffer[0] != STM32_ACK)
		return STM32_ERR_PROTOCOL;

	/* read bootloader version */
	if(serial_read(fd, buffer, 1) != SERIAL_ERR_OK)
		return STM32_ERR_SERIAL;

	/* read protection disable counter */
	if(serial_read(fd, rpdc, 1) != SERIAL_ERR_OK)
		return STM32_ERR_SERIAL;

	/* read protection enable counter */
	if(serial_read(fd, rpec, 1) != SERIAL_ERR_OK)
		return STM32_ERR_SERIAL;

	/* read ACK */
	if(serial_read(fd, buffer, 1) != SERIAL_ERR_OK)
		return STM32_ERR_SERIAL;

	if (buffer[0] != STM32_ACK)
		return STM32_ERR_PROTOCOL;

	return STM32_ERR_OK;
}

stm32_errors_t stm32_get_id(int fd, uint8_t **device_id, uint8_t *device_id_size){

	static uint8_t id[0x100]; // 0xFF + 1

	uint8_t buffer[0xFF];
	uint8_t len;

	/* send 'get id' command and read response */
	stm32_errors_t result = send_cmd(fd, STM32_GET_ID, buffer);

	if (result != STM32_ERR_OK)
		return result;

	if (buffer[0] != STM32_ACK)
		return STM32_ERR_PROTOCOL;

	/* read size of response */
	if(serial_read(fd, buffer, 1) != SERIAL_ERR_OK)
		return STM32_ERR_SERIAL;

	if (buffer[0] < 0)
		return STM32_ERR_PROTOCOL;

	len = buffer[0] + 1;

	/* read device id */
	if(serial_read(fd, id, len) != SERIAL_ERR_OK)
		return STM32_ERR_SERIAL;

	*device_id = id;
	*device_id_size = len;

	/* read ACK */
	if(serial_read(fd, buffer, 1) != SERIAL_ERR_OK)
		return STM32_ERR_SERIAL;

	if (buffer[0] != STM32_ACK)
		return STM32_ERR_PROTOCOL;

	return STM32_ERR_OK;
}

stm32_errors_t stm32_read(int fd, uint32_t start_address, uint8_t **data, uint16_t data_size){

	static uint8_t response[0x100]; //0xFF + 1

	uint8_t buffer[0xFF];

	if(data_size == 0 || data_size > 0x100)	//0xFF + 1
		return STM32_ERR_INVALID_ARGUMENT;

	/* send 'read memory' command and read response */
	stm32_errors_t result = send_cmd(fd, STM32_READ, buffer);

	if (result != STM32_ERR_OK)
		return result;

	/* check for Read Device Protection */
	if (buffer[0] == STM32_NACK)
		return STM32_ERR_RDP;

	if (buffer[0] != STM32_ACK)
		return STM32_ERR_PROTOCOL;

	/* build start address with checksum */
	buffer[0] = (start_address >> 24) & 0xFF;
	buffer[1] = (start_address >> 16) & 0xFF;
	buffer[2] = (start_address >> 8) & 0xFF;
	buffer[3] = (start_address >> 0) & 0xFF;

	buffer[4] = buffer[0] ^ buffer[1] ^ buffer[2] ^ buffer[3];

	/* send start address with checksum and wait ACK */
	if(
		serial_write(fd, buffer, 5) != SERIAL_ERR_OK ||
		serial_read(fd, buffer, 1) != SERIAL_ERR_OK
	)
		return STM32_ERR_SERIAL;

	/* check for bad address */
	if (buffer[0] == STM32_NACK)
		return STM32_ERR_INVALID_ARGUMENT;

	if (buffer[0] != STM32_ACK)
		return STM32_ERR_PROTOCOL;

	/* calculate block size and checksum */
	buffer[0] = data_size - 1;
	buffer[1] = buffer[0] ^ 0xFF;

	/* send block size with checksum */
	if(
		serial_write(fd, buffer, 2) != SERIAL_ERR_OK ||
		serial_read(fd, buffer, 1) != SERIAL_ERR_OK
	)
		return STM32_ERR_SERIAL;

	/* check for Read Device Protection */
	if (buffer[0] == STM32_NACK)
		return STM32_ERR_RDP;

	if (buffer[0] != STM32_ACK)
		return STM32_ERR_PROTOCOL;

	/* read data */
	if(serial_read(fd, response, data_size) != SERIAL_ERR_OK)
		return STM32_ERR_SERIAL;

	*data = response;

	return STM32_ERR_OK;
}

stm32_errors_t stm32_write(int fd, uint32_t start_address, const uint8_t *data, uint16_t data_size){

	uint8_t buffer[0x200];
	uint8_t check_summ;

	if(data_size == 0 || data_size > 0x100)	//0xFF + 1
		return STM32_ERR_INVALID_ARGUMENT;

	/* check for align */
	if((start_address % 4) != 0)
		return STM32_ERR_INVALID_ARGUMENT;

	/* send 'write memory' command and read response */
	stm32_errors_t result = send_cmd(fd, STM32_WRITE, buffer);

	if (result != STM32_ERR_OK)
		return result;

	/* check for Read Device Protection */
	if (buffer[0] == STM32_NACK)
		return STM32_ERR_RDP;

	if (buffer[0] != STM32_ACK)
		return STM32_ERR_PROTOCOL;

	/* build start address with checksum */
	buffer[0] = (start_address >> 24) & 0xFF;
	buffer[1] = (start_address >> 16) & 0xFF;
	buffer[2] = (start_address >> 8) & 0xFF;
	buffer[3] = (start_address >> 0) & 0xFF;

	buffer[4] = buffer[0] ^ buffer[1] ^ buffer[2] ^ buffer[3];

	/* send start address with checksum and wait ACK */
	if(
		serial_write(fd, buffer, 5) != SERIAL_ERR_OK ||
		serial_read(fd, buffer, 1) != SERIAL_ERR_OK
	)
		return STM32_ERR_SERIAL;

	/* check for bad address */
	if (buffer[0] == STM32_NACK)
		return STM32_ERR_INVALID_ARGUMENT;

	if (buffer[0] != STM32_ACK)
		return STM32_ERR_PROTOCOL;

	/* calculate block size and checksum */
	buffer[0] = data_size - 1;

	int i = 0;
	check_summ = buffer[0];

	for(i = 0; i < data_size; i++){
		buffer[i + 1] = data[i];
		check_summ ^= data[i];
	}
	buffer[i + 1] = check_summ;

	/* send data */
	if(
		serial_write(fd, buffer, data_size + 2) != SERIAL_ERR_OK ||
		serial_read(fd, buffer, 1) != SERIAL_ERR_OK
	)
		return STM32_ERR_SERIAL;

	if (buffer[0] != STM32_ACK)
		return STM32_ERR_PROTOCOL;

	return STM32_ERR_OK;

}

stm32_errors_t stm32_extended_erase(int fd, const uint16_t *pages, uint16_t pages_size){

	uint8_t buffer[0xFF];
	uint8_t check_summ;

	if(pages_size == 0)
		return STM32_ERR_INVALID_ARGUMENT;

	/* check for special commands */
	if(pages_size - 1 == STM32_EE_ERASE_MASS || pages_size - 1 == STM32_EE_ERASE_BANK1 || pages_size - 1 == STM32_EE_ERASE_BANK2)
		return STM32_ERR_INVALID_ARGUMENT;

	/* send 'extended erase memory' command and read response */
	stm32_errors_t result = send_cmd(fd, STM32_EXTENDED_ERASE, buffer);

	if (result != STM32_ERR_OK)
		return result;

	/* check for Read Device Protection */
	if (buffer[0] == STM32_NACK)
		return STM32_ERR_RDP;

	if (buffer[0] != STM32_ACK)
		return STM32_ERR_PROTOCOL;

	/* send number of pages to be erased */
	buffer[0] = ((pages_size - 1) >> 8) & 0xFF;
	buffer[1] = ((pages_size - 1) >> 0) & 0xFF;

	check_summ = buffer[0];
	check_summ ^= buffer[1];

	if(serial_write(fd, buffer, 2) != SERIAL_ERR_OK)
		return STM32_ERR_SERIAL;

	uint16_t i = 0;

	/* send pages number */
	for(i = 0; i < pages_size; i++){

		buffer[0] = (pages[i] >> 8) & ~((~0) << 8);
		buffer[1] = (pages[i] >> 0) & ~((~0) << 8);

		check_summ ^= buffer[0];
		check_summ ^= buffer[1];

		if(serial_write(fd, buffer, 2) != SERIAL_ERR_OK)
			return STM32_ERR_SERIAL;

	}

	/* send check sum */
	if(serial_write(fd, &check_summ, 1) != SERIAL_ERR_OK)
		return STM32_ERR_SERIAL;

	/* wait ACK */
	while(serial_read(fd, buffer, 1) != SERIAL_ERR_OK);

	if (buffer[0] != STM32_ACK)
		return STM32_ERR_PROTOCOL;

	return STM32_ERR_OK;
}

stm32_errors_t stm32_extended_erase_special(int fd, stm32_erase_type_t erase_type){

	uint8_t buffer[0xFF];
	uint8_t check_summ;
	uint16_t cmd;

	switch (erase_type) {
		case STM32_ERASE_MASS: cmd = STM32_EE_ERASE_MASS; break;
		case STM32_ERASE_BANK1: cmd = STM32_EE_ERASE_BANK1; break;
		case STM32_ERASE_BANK2: cmd = STM32_EE_ERASE_BANK2; break;
		default:
			return STM32_ERR_INVALID_ARGUMENT;
	}

	/* send 'extended erase memory' command and read response */
	stm32_errors_t result = send_cmd(fd, STM32_EXTENDED_ERASE, buffer);

	if (result != STM32_ERR_OK)
		return result;

	/* check for Read Device Protection */
	if (buffer[0] == STM32_NACK)
		return STM32_ERR_RDP;

	if (buffer[0] != STM32_ACK)
		return STM32_ERR_PROTOCOL;

	/* send special erase command */
	buffer[0] = (cmd >> 8) & 0xFF;
	buffer[1] = (cmd >> 0) & 0xFF;

	check_summ = buffer[0];
	check_summ ^= buffer[1];

	if(serial_write(fd, buffer, 2) != SERIAL_ERR_OK)
		return STM32_ERR_SERIAL;

	/* send check sum */
	if(serial_write(fd, &check_summ, 1) != SERIAL_ERR_OK)
		return STM32_ERR_SERIAL;

	/* wait ACK or NACK */
	while(serial_read(fd, buffer, 1) != SERIAL_ERR_OK);

	if (buffer[0] != STM32_ACK)
		return STM32_ERR_PROTOCOL;

	return STM32_ERR_OK;
}

stm32_errors_t stm32_write_protect(int fd, const uint8_t *pages, uint16_t pages_size){

	uint8_t buffer[0xFF];
	uint8_t check_summ;

	if(pages_size == 0 || pages_size > 0x100)	//0xFF + 1
		return STM32_ERR_INVALID_ARGUMENT;

	/* send 'write protect' command and read response */
	stm32_errors_t result = send_cmd(fd, STM32_WRITE_PROTECT, buffer);

	if (result != STM32_ERR_OK)
		return result;

	/* check for Read Device Protection */
	if (buffer[0] == STM32_NACK)
		return STM32_ERR_RDP;

	if (buffer[0] != STM32_ACK)
		return STM32_ERR_PROTOCOL;

	/* send number of pages */
	buffer[0] = (pages_size - 1) & 0xFF;
	check_summ = buffer[0];

	if(serial_write(fd, buffer, 1) != SERIAL_ERR_OK)
		return STM32_ERR_SERIAL;

	/* send pages */
	if(serial_write(fd, pages, pages_size) != SERIAL_ERR_OK)
		return STM32_ERR_SERIAL;

	/* send check sum */
	uint16_t i = 0;
	for(i=0; i < pages_size; i++){
		check_summ ^= pages[i];
	}

	if(serial_write(fd, &check_summ, 1) != SERIAL_ERR_OK)
		return STM32_ERR_SERIAL;

	/* read ACK */
	if(serial_read(fd, buffer, 1) != SERIAL_ERR_OK)
		return STM32_ERR_SERIAL;

	if (buffer[0] != STM32_ACK)
		return STM32_ERR_PROTOCOL;

	return STM32_ERR_OK;

}

stm32_errors_t stm32_write_unprotect(int fd){

	uint8_t buffer[0xFF];

	/* send 'write unprotect' command and read response */
	stm32_errors_t result = send_cmd(fd, STM32_WRITE_UNPROTECT, buffer);

	if (result != STM32_ERR_OK)
		return result;

	/* check for Read Device Protection */
	if (buffer[0] == STM32_NACK)
		return STM32_ERR_RDP;

	if (buffer[0] != STM32_ACK)
		return STM32_ERR_PROTOCOL;

	/* read ACK */
	if(serial_read(fd, buffer, 1) != SERIAL_ERR_OK)
		return STM32_ERR_SERIAL;

	if (buffer[0] != STM32_ACK)
		return STM32_ERR_PROTOCOL;

	return STM32_ERR_OK;
}

stm32_errors_t stm32_readout_protect(int fd){

	uint8_t buffer[0xFF];

	/* send 'readout protect' command and read response */
	stm32_errors_t result = send_cmd(fd, STM32_READOUT_PROTECT, buffer);

	if (result != STM32_ERR_OK)
		return result;

	/* check for Read Device Protection */
	if (buffer[0] == STM32_NACK)
		return STM32_ERR_RDP;

	if (buffer[0] != STM32_ACK)
		return STM32_ERR_PROTOCOL;

	/* read ACK */
	if(serial_read(fd, buffer, 1) != SERIAL_ERR_OK)
		return STM32_ERR_SERIAL;

	if (buffer[0] != STM32_ACK)
		return STM32_ERR_PROTOCOL;

	return STM32_ERR_OK;

}

stm32_errors_t stm32_readout_unprotect(int fd){

	uint8_t buffer[0xFF];

	/* send 'readout unprotect' command and read response */
	stm32_errors_t result = send_cmd(fd, STM32_READOUT_UNPROTECT, buffer);

	if (result != STM32_ERR_OK)
		return result;

	if (buffer[0] != STM32_ACK)
		return STM32_ERR_PROTOCOL;

	/* read ACK */
	if(serial_read(fd, buffer, 1) != SERIAL_ERR_OK)
		return STM32_ERR_SERIAL;

	if (buffer[0] != STM32_ACK)
		return STM32_ERR_PROTOCOL;

	return STM32_ERR_OK;
}
