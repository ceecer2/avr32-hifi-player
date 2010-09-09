/*
 * user_board.h
 *
 *  Created on: 2010-4-17
 *      Author: Kyle
 */

#ifndef USER_BOARD_H_
#define USER_BOARD_H_

//#define FOSC0	11289600
//#define FOSC1	12288000
#define FOSC0	12000000
#define FOSC1	16000000
#define FCPU	60000000
#define FPBA	FCPU

#define SD_MMC_SPI			(&AVR32_SPI)
#define SD_MMC_SPI_NPCS		0
#define DATA_FLASH_CS		1

#define PDCA_CHANNEL_SSC	0
#define PDCA_CHANNEL_SPI_RX	1
#define PDCA_CHANNEL_SPI_TX	2

#define RT_DEVICE_CTRL_SYSCLOCK_CHANGED		0x80

#include <rtthread.h>

extern rt_sem_t gSPIMutex;
extern rt_uint32_t gPBAFreq;

void rt_hw_board_init();
void peripherials_init();

#endif /* USER_BOARD_H_ */
