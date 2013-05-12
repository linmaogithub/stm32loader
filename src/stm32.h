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
