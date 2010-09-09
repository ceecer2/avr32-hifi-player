/*
 * board.c
 *
 *  Created on: 2010-4-17
 *      Author: Kyle
 */

#include <compiler.h>
#include <pm.h>
#include <gpio.h>
#include <spi.h>
#include <usart.h>
#include <eic.h>
#include <intc.h>
#include <pdca.h>
#include <tc.h>

#include <rtthread.h>
#include <board.h>
#include "lcd/ssd1283a.h"
#include "console/usart_console.h"

rt_sem_t    gSPIMutex;
rt_uint32_t gPBAFreq;

static rt_uint32_t counter_compare;

/**
 * System tick interrupt handler.
 */
static void rt_hw_timer_handler(void)
{
	// Clears the interrupt request.
	Set_system_register(AVR32_COMPARE, counter_compare);

	rt_tick_increase();
}

void peripherials_init()
{
	/*
	 * PM initialization: OSC0 = 12MHz XTAL, PLL0 = 60MHz System Clock
	 */
	pm_freq_param_t pm_freq_param =
	{
		.cpu_f = FCPU,
		.pba_f = FPBA,
		.osc0_f = FOSC0,
		.osc0_startup = AVR32_PM_OSCCTRL0_STARTUP_2048_RCOSC
	};
	pm_configure_clocks(&pm_freq_param);
	pm_gc_setup(&AVR32_PM, 0, AVR32_GC_USES_OSC, AVR32_GC_USES_OSC0, AVR32_GC_NO_DIV_CLOCK, 0);
	pm_gc_enable(&AVR32_PM, 0);
	gPBAFreq = pm_freq_param.pba_f;

	/*
	 * GPIO initialization.
	 */
	static const gpio_map_t SPI_GPIO_MAP =
	{
		/* SPI */
		{AVR32_SPI_MISO_0_0_PIN,	AVR32_SPI_MISO_0_0_FUNCTION},
		{AVR32_SPI_MOSI_0_0_PIN,	AVR32_SPI_MOSI_0_0_FUNCTION},
		{AVR32_SPI_SCK_0_1_PIN, 	AVR32_SPI_SCK_0_1_FUNCTION},
		{AVR32_SPI_NPCS_0_0_PIN,	AVR32_SPI_NPCS_0_0_FUNCTION},
		{AVR32_SPI_NPCS_1_1_PIN,	AVR32_SPI_NPCS_1_1_FUNCTION},

		/* SSC */
		{AVR32_PM_GCLK_0_PIN,			AVR32_PM_GCLK_0_FUNCTION},
		{AVR32_SSC_TX_CLOCK_0_PIN,		AVR32_SSC_TX_CLOCK_0_FUNCTION},
		{AVR32_SSC_TX_DATA_0_PIN,		AVR32_SSC_TX_DATA_0_FUNCTION},
		{AVR32_SSC_TX_FRAME_SYNC_0_PIN, AVR32_SSC_TX_FRAME_SYNC_0_FUNCTION},

		/* USART2 */
		{AVR32_USART2_TXD_0_1_PIN,	AVR32_USART2_TXD_0_1_FUNCTION},
		{AVR32_USART2_CLK_0_PIN,	AVR32_USART2_CLK_0_FUNCTION},
		{AVR32_USART2_RTS_0_PIN,	AVR32_USART2_RTS_0_FUNCTION},

		/* EIC */
		{AVR32_EIC_SCAN_0_PIN,		AVR32_EIC_SCAN_0_FUNCTION},
		{AVR32_EIC_SCAN_1_PIN,		AVR32_EIC_SCAN_1_FUNCTION},
		{AVR32_EIC_SCAN_2_PIN,		AVR32_EIC_SCAN_2_FUNCTION},
		{AVR32_EIC_EXTINT_0_PIN,	AVR32_EIC_EXTINT_0_FUNCTION},
		{AVR32_EIC_EXTINT_1_PIN,	AVR32_EIC_EXTINT_1_FUNCTION},
	};
	gpio_enable_module(SPI_GPIO_MAP, sizeof(SPI_GPIO_MAP) / sizeof(SPI_GPIO_MAP[0]));

	/* LCD_DC */
	gpio_set_gpio_pin(SSD1283A_DC_PIN);

	/*
	 * SPI Initialization.
	 */
	spi_options_t spi_options =
	{
		.reg = SD_MMC_SPI_NPCS,
		.baudrate = FPBA / 3,			/* Approx. 20M */
		.bits = 8,
		.spck_delay = 0,
		.trans_delay = 0,
		.stay_act = 1,
		.spi_mode = 0,
		.modfdis = 1
	};
	spi_initMaster(&AVR32_SPI, &spi_options);
	spi_selectionMode(&AVR32_SPI, 0, 0, 0);

	/* SD_MMC Channel */
	spi_setupChipReg(&AVR32_SPI, &spi_options, FPBA);

	/* DataFlash Channel */
	spi_options.reg = DATA_FLASH_CS;
	spi_options.baudrate = FPBA / 2;	/* Approx. 30M */
	spi_setupChipReg(&AVR32_SPI, &spi_options, FPBA);

	spi_enable(&AVR32_SPI);

	/*
	 * USART2 Initialization.
	 */
	static const usart_spi_options_t USART_SPI_OPTIONS =
	{
		.baudrate = FPBA / 8,			/* Approx. 7.5M */
		.charlength = 8,
		.spimode = 0,
		.channelmode = USART_NORMAL_CHMODE
	};
	usart_init_spi_master(SSD1283A_USART_PORT, &USART_SPI_OPTIONS, FPBA);

	/*
	 * EIC Initialization.
	 */
	gpio_enable_pin_pull_up(AVR32_EIC_EXTINT_0_PIN);
	gpio_enable_pin_pull_up(AVR32_EIC_EXTINT_1_PIN);
	AVR32_EIC.filter = AVR32_EIC_INT0_MASK | AVR32_EIC_INT1_MASK;
	AVR32_EIC.en = AVR32_EIC_INT0_MASK | AVR32_EIC_INT1_MASK;
	AVR32_EIC.ier = AVR32_EIC_INT0_MASK | AVR32_EIC_INT1_MASK;
	AVR32_EIC.scan = AVR32_EIC_SCAN_EN_MASK | (11 << AVR32_EIC_SCAN_PRESC_OFFSET);

	/*
	 * PDCA Initialization.
	 */
	pdca_channel_options_t pdca_channel_options =
	{
		.addr = NULL,
		.size = 0,
		.r_addr = NULL,
		.r_size = 0,
		.pid = AVR32_PDCA_PID_SSC_TX,
		.transfer_size = PDCA_TRANSFER_SIZE_HALF_WORD
	};
	pdca_init_channel(PDCA_CHANNEL_SSC, &pdca_channel_options);

	pdca_channel_options.pid = AVR32_PDCA_PID_SPI_RX;
	pdca_channel_options.transfer_size = PDCA_TRANSFER_SIZE_BYTE;
	pdca_init_channel(PDCA_CHANNEL_SPI_RX, &pdca_channel_options);

	pdca_channel_options.pid = AVR32_PDCA_PID_SPI_TX;
	pdca_channel_options.transfer_size = PDCA_TRANSFER_SIZE_BYTE;
	pdca_init_channel(PDCA_CHANNEL_SPI_TX, &pdca_channel_options);

	/*
	 * INTC Initialization.
	 */
	INTC_init_interrupts();
//	INTC_register_interrupt(&eic_int_handler, AVR32_EIC_IRQ_0, AVR32_INTC_INT0);
//	INTC_register_interrupt(&eic_int_handler, AVR32_EIC_IRQ_1, AVR32_INTC_INT0);
//	INTC_register_interrupt(&ssc_tx_pdca_rcz_int_handler, AVR32_PDCA_IRQ_0 + PDCA_CHANNEL_SSC);
//	INTC_register_interrupt(&spi_rx_pdca_tc_int_handler, AVR32_PDCA_IRQ_0 + PDCA_CHANNEL_SPI_RX);
//	INTC_register_interrupt(&spi_tx_pdca_tc_int_handler, AVR32_PDCA_IRQ_0 + PDCA_CHANNEL_SPI_TX);
}

/**
 * Initialize CPU cycle counter for system tick.
 */
static void cpu_counter_init(void)
{
	counter_compare = gPBAFreq / RT_TICK_PER_SECOND;
	INTC_register_interrupt(&rt_hw_timer_handler, AVR32_CORE_COMPARE_IRQ, AVR32_INTC_INT3);
	Set_system_register(AVR32_COMPARE, counter_compare);
	Set_system_register(AVR32_COUNT, 0);
}

void rt_hw_board_init()
{
	peripherials_init();
	usart_console_init();
	cpu_counter_init();
}
