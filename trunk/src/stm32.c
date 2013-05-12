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

#define STM32_INIT		0x7F
#define STM32_ACK		0x79
#define STM32_NACK		0x1F
#define STM32_GET		0x00
#define STM32_GET_RPS	0x01
#define STM32_GET_ID	0x02
#define STM32_READ		0x11
#define STM32_WRITE		0x31
#define STM32_EXTENDED_ERASE		0x44

#define READ_BLOCK_SIZE		0x100U
#define WRITE_BLOCK_SIZE	0x100U

stm32_errors_t stm32_init(int fd) {

	uint8_t cmd = STM32_INIT;

	/* send 'init' command and wait ACK */
	if (
		serial_write(fd, &cmd, 1) != SERIAL_ERR_OK ||
		serial_read(fd, &cmd, 1) != SERIAL_ERR_OK
	)
		return STM32_ERR_SERIAL;

	if (cmd != STM32_ACK)
		return STM32_ERR_PROTOCOL;

	return STM32_ERR_OK;
}

stm32_errors_t stm32_get_bootloader_version(int fd, uint8_t *version){

	uint8_t cmd[2];
	uint8_t buffer[255];
	uint8_t len;

	/* build 'get' command */
	cmd[0] = STM32_GET;
	cmd[1] = cmd[0] ^ 0xFF;

	/* send 'get' command and wait ACK */
	if(
		serial_write(fd, cmd, 2) != SERIAL_ERR_OK ||
		serial_read(fd, buffer, 1) != SERIAL_ERR_OK
	)
		return STM32_ERR_SERIAL;

	if (buffer[0] != STM32_ACK)
		return STM32_ERR_PROTOCOL;

	/* read size of response */
	if(serial_read(fd, buffer, 1) != SERIAL_ERR_OK)
		return STM32_ERR_SERIAL;

	if (buffer[0] < 0)
		return STM32_ERR_PROTOCOL;

	len = buffer[0];
	len += 1;

	/* read bootloader version */
	if(serial_read(fd, version, 1) != SERIAL_ERR_OK)
		return STM32_ERR_SERIAL;

	len--;

	/* read supported commands */
	if(serial_read(fd, buffer, len) != SERIAL_ERR_OK)
		return STM32_ERR_SERIAL;

	/* read ACK */
	if(serial_read(fd, buffer, 1) != SERIAL_ERR_OK)
		return STM32_ERR_SERIAL;

	if (buffer[0] != STM32_ACK)
		return STM32_ERR_PROTOCOL;

	return STM32_ERR_OK;
}

stm32_errors_t stm32_get_supported_commands(int fd, uint8_t *commands, int buffer_size, uint8_t *size){

	uint8_t cmd[2];
	uint8_t buffer[255];
	uint8_t len;

	/* build 'get' command */
	cmd[0] = STM32_GET;
	cmd[1] = cmd[0] ^ 0xFF;

	/* send 'get' command and wait ACK */
	if(
		serial_write(fd, cmd, 2) != SERIAL_ERR_OK ||
		serial_read(fd, buffer, 1) != SERIAL_ERR_OK
	)
		return STM32_ERR_SERIAL;

	if (buffer[0] != STM32_ACK)
		return STM32_ERR_PROTOCOL;

	/* read size of response */
	if(serial_read(fd, buffer, 1) != SERIAL_ERR_OK)
		return STM32_ERR_SERIAL;

	if (buffer[0] < 0)
		return STM32_ERR_PROTOCOL;

	len = buffer[0];
	len += 1;

	/* read bootloader version */
	if(serial_read(fd, buffer, 1) != SERIAL_ERR_OK)
		return STM32_ERR_SERIAL;

	len--;

	if (buffer_size < len)
		return STM32_ERR_INVALID_ARGUMENT;

	/* read supported commands */
	if(serial_read(fd, commands, len) != SERIAL_ERR_OK)
		return STM32_ERR_SERIAL;

	*size = len;

	/* read ACK */
	if(serial_read(fd, buffer, 1) != SERIAL_ERR_OK)
		return STM32_ERR_SERIAL;

	if (buffer[0] != STM32_ACK)
		return STM32_ERR_PROTOCOL;

	return STM32_ERR_OK;
}

stm32_errors_t stm32_get_prs(int fd, uint8_t *rpdc, uint8_t *rpec){

	uint8_t cmd[2];
	uint8_t buffer[255];

	/* build 'get prs' command */
	cmd[0] = STM32_GET_RPS;
	cmd[1] = cmd[0] ^ 0xFF;

	/* send 'get prs' command and wait ACK */
	if(
		serial_write(fd, cmd, 2) != SERIAL_ERR_OK ||
		serial_read(fd, buffer, 1) != SERIAL_ERR_OK
	)
		return STM32_ERR_SERIAL;

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

stm32_errors_t stm32_get_id(int fd, uint8_t *id, int buffer_size, uint8_t *size){

	uint8_t cmd[2];
	uint8_t buffer[255];
	uint8_t len;

	/* build 'get id' command */
	cmd[0] = STM32_GET_ID;
	cmd[1] = cmd[0] ^ 0xFF;

	/* send 'get id' command and wait ACK */
	if(
		serial_write(fd, cmd, 2) != SERIAL_ERR_OK ||
		serial_read(fd, buffer, 1) != SERIAL_ERR_OK
	)
		return STM32_ERR_SERIAL;

	if (buffer[0] != STM32_ACK)
		return STM32_ERR_PROTOCOL;

	/* read size of response */
	if(serial_read(fd, buffer, 1) != SERIAL_ERR_OK)
		return STM32_ERR_SERIAL;

	if (buffer[0] < 0)
		return STM32_ERR_PROTOCOL;

	len = buffer[0] + 1;

	if (buffer_size < len)
		return STM32_ERR_INVALID_ARGUMENT;

	/* read device id */
	if(serial_read(fd, id, len) != SERIAL_ERR_OK)
		return STM32_ERR_SERIAL;

	*size = len;

	/* read ACK */
	if(serial_read(fd, buffer, 1) != SERIAL_ERR_OK)
		return STM32_ERR_SERIAL;

	if (buffer[0] != STM32_ACK)
		return STM32_ERR_PROTOCOL;

	return STM32_ERR_OK;
}

stm32_errors_t stm32_read(int fd, const uint32_t start_address, void *data, const uint32_t data_size){

	uint8_t cmd[2];
	uint8_t buffer[255];
	uint8_t *data_ptr = (uint8_t*)data;
	uint16_t block_size;
	uint32_t readed = 0;

	/* build 'read memory' command */
	cmd[0] = STM32_READ;
	cmd[1] = cmd[0] ^ 0xFF;

	while(readed < data_size){

		/* send 'read memory' command and wait ACK */
		if(
			serial_write(fd, &cmd, 2) != SERIAL_ERR_OK ||
			serial_read(fd, &buffer, 1) != SERIAL_ERR_OK
		)
			return STM32_ERR_SERIAL;

		/* check for Read Device Protection */
		if (buffer[0] == STM32_NACK)
			return STM32_ERR_RDP;

		if (buffer[0] != STM32_ACK)
			return STM32_ERR_PROTOCOL;

		/* build start address with checksum */
		buffer[0] = ((start_address + readed) >> 24) & ~((~0) << 8);
		buffer[1] = ((start_address + readed) >> 16) & ~((~0) << 8);
		buffer[2] = ((start_address + readed) >> 8) & ~((~0) << 8);
		buffer[3] = ((start_address + readed) >> 0) & ~((~0) << 8);

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
		if((data_size - readed) >= READ_BLOCK_SIZE)
			block_size = READ_BLOCK_SIZE;
		else
			block_size = data_size - readed;

		buffer[0] = block_size - 1;
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
		if(serial_read(fd, data_ptr + readed, block_size) != SERIAL_ERR_OK)
			return STM32_ERR_SERIAL;

		readed += block_size;
	}

	return STM32_ERR_OK;
}

stm32_errors_t stm32_write(int fd, const uint32_t start_address, void *data, const uint32_t data_size){

	uint8_t cmd[2];
	uint8_t buffer[512];
	uint8_t *data_ptr = (uint8_t*)data;
	uint16_t block_size;
	uint32_t writed = 0;
	uint8_t check_summ;

	/* build 'write memory' command */
	cmd[0] = STM32_WRITE;
	cmd[1] = cmd[0] ^ 0xFF;

	while(writed < data_size){

		/* check for align */
		if((start_address + writed) % 4 != 0)
			return STM32_ERR_INVALID_ARGUMENT;

		/* send 'write memory' command and wait ACK */
		if(
			serial_write(fd, cmd, 2) != SERIAL_ERR_OK ||
			serial_read(fd, buffer, 1) != SERIAL_ERR_OK
		)
			return STM32_ERR_SERIAL;

		/* check for Read Device Protection */
		if (buffer[0] == STM32_NACK)
			return STM32_ERR_RDP;

		if (buffer[0] != STM32_ACK)
			return STM32_ERR_PROTOCOL;

		/* build start address with checksum */
		buffer[0] = ((start_address + writed) >> 24) & ~((~0) << 8);
		buffer[1] = ((start_address + writed) >> 16) & ~((~0) << 8);
		buffer[2] = ((start_address + writed) >> 8) & ~((~0) << 8);
		buffer[3] = ((start_address + writed) >> 0) & ~((~0) << 8);

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
		if((data_size - writed) >= WRITE_BLOCK_SIZE)
			block_size = WRITE_BLOCK_SIZE;
		else
			block_size = data_size - writed;

		buffer[0] = block_size - 1;
		buffer[1] = buffer[0] ^ 0xFF;

		int i = 0;
		check_summ = buffer[0];

		for(i = 0; i < block_size; i++){
			buffer[i + 1] = data_ptr[writed + i];
			check_summ ^= buffer[i + 1];
		}
		buffer[i + 1] = check_summ;

		/* send data */
		if(
			serial_write(fd, buffer, block_size + 2) != SERIAL_ERR_OK ||
			serial_read(fd, buffer, 1) != SERIAL_ERR_OK
		)
			return STM32_ERR_SERIAL;

		if (buffer[0] != STM32_ACK)
			return STM32_ERR_PROTOCOL;

		writed += block_size;

	}

	return STM32_ERR_OK;

}

stm32_errors_t stm32_extended_erase(int fd, const uint8_t *pages, const uint8_t buffer_size){

	uint8_t cmd[2];
	uint8_t buffer[512];
	uint8_t check_summ;

	/* check for special value */
	if(buffer_size == 0xFF)
		return STM32_ERR_INVALID_ARGUMENT;

	/* build 'extended erase memory' command */
	cmd[0] = STM32_EXTENDED_ERASE;
	cmd[1] = cmd[0] ^ 0xFF;

	/* send 'erase memory' command and wait ACK */
	if(
		serial_write(fd, cmd, 2) != SERIAL_ERR_OK ||
		serial_read(fd, buffer, 1) != SERIAL_ERR_OK
	)
		return STM32_ERR_SERIAL;

	/* check for Read Device Protection */
	if (buffer[0] == STM32_NACK)
		return STM32_ERR_RDP;

	if (buffer[0] != STM32_ACK)
		return STM32_ERR_PROTOCOL;

	buffer[0] = buffer_size;

	int i = 0;
	check_summ = buffer[0];

	for(i = 0; i < buffer_size; i++){
		buffer[i + 1] = pages[i];
		check_summ ^= buffer[i + 1];
	}
	buffer[i + 1] = check_summ;

	/* read ACK */
	if(serial_read(fd, buffer, 1) != SERIAL_ERR_OK)
		return STM32_ERR_SERIAL;

	if (buffer[0] != STM32_ACK)
		return STM32_ERR_PROTOCOL;

	return STM32_ERR_OK;
}
