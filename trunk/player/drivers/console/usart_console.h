/*
 * usart_console.h
 *
 *  Created on: 2010-4-23
 *      Author: Kyle
 */

#ifndef USART_CONSOLE_H_
#define USART_CONSOLE_H_

#include <compiler.h>
#include <usart.h>
#include <gpio.h>
#include <intc.h>
#include <board.h>

#include <rthw.h>
#include <rtthread.h>

#define CONSOLE_PORT				(&AVR32_USART1)
#define CONSOLE_PORT_TXD_PIN		AVR32_USART1_TXD_0_1_PIN
#define CONSOLE_PORT_TXD_FUNCTION	AVR32_USART1_TXD_0_1_FUNCTION
#define CONSOLE_PORT_RXD_PIN		AVR32_USART1_RXD_0_1_PIN
#define CONSOLE_PORT_RXD_FUNCTION	AVR32_USART1_RXD_0_1_FUNCTION
#define CONSOLE_PORT_IRQ			AVR32_USART1_IRQ
#define CONSOLE_PORT_IRQ_LEVEL		AVR32_INTC_INT0

#define CONSOLE_DEVICE_NAME			"console"

#define RX_BUFFER_SIZE				16

void usart_console_init();

#endif /* USART_CONSOLE_H_ */
