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

typedef enum stm32_erase_type {
	STM32_ERASE_MASS,
	STM32_ERASE_BANK1,
	STM32_ERASE_BANK2
} stm32_erase_type_t ;

stm32_errors_t stm32_init(int fd);
stm32_errors_t stm32_get(int fd, uint8_t *version, uint8_t **supported_commands, uint8_t *supported_commands_size);
stm32_errors_t stm32_get_prs(int fd, uint8_t *rpdc, uint8_t *rpec);
stm32_errors_t stm32_get_id(int fd, uint8_t **device_id, uint8_t *device_id_size);
stm32_errors_t stm32_read(int fd, uint32_t start_address, uint8_t **data, uint16_t data_size);
stm32_errors_t stm32_write(int fd, uint32_t start_address, const uint8_t *data, uint16_t data_size);
stm32_errors_t stm32_extended_erase(int fd, const uint16_t *pages, uint16_t pages_size);
stm32_errors_t stm32_extended_erase_special(int fd, stm32_erase_type_t erase_type);
stm32_errors_t stm32_write_protect(int fd, const uint8_t *pages, uint16_t pages_size);
stm32_errors_t stm32_write_unprotect(int fd);
stm32_errors_t stm32_readout_protect(int fd);
stm32_errors_t stm32_readout_unprotect(int fd);

#endif /* STM32_H_ */
