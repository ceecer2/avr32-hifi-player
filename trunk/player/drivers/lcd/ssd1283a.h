/*
 * ssd1283a.h
 *
 *  Created on: 2010-4-14
 *      Author: Kyle
 */

#ifndef SSD1283A_H_
#define SSD1283A_H_

#include <stdint.h>

// SPI: 0, USART: 1
#define SSD1283A_INTERFACE		1

#define SSD1283A_DC_PIN			AVR32_PIN_PA22

#if SSD1283A_INTERFACE == 0

#define SSD1283A_SPI_CS			2
#define SSD1283A_SPI_PORT		(&AVR32_SPI)

#else

#define SSD1283A_USART_PORT		(&AVR32_USART2)

#endif

#define SSD1283A_MAX_X			130
#define SSD1283A_MAX_Y			130
#define SSD1283A_COL_OFFSET		2
#define SSD1283A_ROW_OFFSET		0
#define SSD1283A_PIXEL_BYTES	2

void rt_hw_ssd1283a_init();

#endif /* SSD1283A_H_ */
