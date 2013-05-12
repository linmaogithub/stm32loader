/*
 * stm32.h
 *
 *  Created on: May 2, 2013
 *      Author: jay
 */

#ifndef STM32_H_
#define STM32_H_

#include <stdint.h>

typedef enum stm32_errors {
	STM32_ERR_OK,
	STM32_ERR_SERIAL,
	STM32_ERR_PROTOCOL,
	STM32_ERR_INVALID_ARGUMENT,
	STM32_ERR_RDP,
} stm32_errors_t ;

stm32_errors_t stm32_init(int fd);

stm32_errors_t stm32_get_bootloader_version(int fd, uint8_t *version);
stm32_errors_t stm32_get_supported_commands(int fd, uint8_t *commands, int buffer_size, uint8_t *size);
stm32_errors_t stm32_get_prs(int fd, uint8_t *rpdc, uint8_t *rpec);
stm32_errors_t stm32_get_id(int fd, uint8_t *id, int buffer_size, uint8_t *size);
stm32_errors_t stm32_read(int fd, const uint32_t start_address, void *data, const uint32_t data_size);
stm32_errors_t stm32_write(int fd, const uint32_t start_address, void *data, const uint32_t data_size);
stm32_errors_t stm32_extended_erase(int fd, const uint8_t *pages, const uint8_t buffer_size);

#endif /* STM32_H_ */
