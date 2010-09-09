/*
 * ssd1283a.c
 *
 *  Created on: 2010-4-14
 *      Author: Kyle
 */

#include <compiler.h>
#include <gpio.h>
#include "ssd1283a.h"

#include <rtgui/rtgui.h>
#include <rtgui/driver.h>
#include <rtgui/color.h>

#define ssd1283a_set_cmd()		(AVR32_GPIO.port[SSD1283A_DC_PIN >> 5].ovrc = 1 << (SSD1283A_DC_PIN & 0x1F))
#define ssd1283a_set_data()		(AVR32_GPIO.port[SSD1283A_DC_PIN >> 5].ovrs = 1 << (SSD1283A_DC_PIN & 0x1F))

#if SSD1283A_INTERFACE == 0

#include <spi.h>

#define ssd1283a_selectChip()	spi_selectChip(SSD1283A_SPI_PORT, SSD1283A_SPI_CS)
#define ssd1283a_unselectChip()	spi_unselectChip(SSD1283A_SPI_PORT, SSD1283A_SPI_CS)
#define ssd1283a_write(data)	spi_write(SSD1283A_SPI_PORT, data)
#define ssd1283a_wait_txempty()	while (!(SSD1283A_SPI_PORT->sr & AVR32_SPI_SR_TXEMPTY_MASK))

#else

#include <usart.h>

#define ssd1283a_selectChip()	SSD1283A_USART_PORT->cr = AVR32_USART_CR_RTSEN_MASK
#define ssd1283a_unselectChip()	SSD1283A_USART_PORT->cr = AVR32_USART_CR_RTSDIS_MASK
#define ssd1283a_write(data)	usart_bw_write_char(SSD1283A_USART_PORT, data)
#define ssd1283a_wait_txempty()	while (!(SSD1283A_USART_PORT->csr & AVR32_USART_CSR_TXEMPTY_MASK))

#endif

static void ssd1283a_wr_reg(uint8_t reg, uint8_t data_h, uint8_t data_l)
{
	ssd1283a_selectChip();

	ssd1283a_write(reg);
	ssd1283a_set_cmd();

	ssd1283a_wait_txempty();
	ssd1283a_write(data_h);
	ssd1283a_set_data();
	ssd1283a_write(data_l);

	ssd1283a_wait_txempty();
	ssd1283a_unselectChip();
}

static void ssd1283a_fill_data(uint8_t data_h, uint8_t data_l)
{
	ssd1283a_write(data_h);
	ssd1283a_write(data_l);
}

void ssd1283a_set_rect(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2)
{
	ssd1283a_wr_reg(0x44, SSD1283A_COL_OFFSET + x2, SSD1283A_COL_OFFSET + x1);
	ssd1283a_wr_reg(0x45, SSD1283A_ROW_OFFSET + y2, SSD1283A_ROW_OFFSET + y1);
}

void ssd1283a_set_pos(uint8_t x, uint8_t y)
{
	ssd1283a_wr_reg(0x21, SSD1283A_ROW_OFFSET + y, SSD1283A_COL_OFFSET + x);
}

void ssd1283a_clear()
{
	volatile int n;

	ssd1283a_set_rect(0, 0, SSD1283A_MAX_X - 1, SSD1283A_MAX_Y - 1);
	ssd1283a_set_pos(0, 0);

	ssd1283a_selectChip();

	ssd1283a_write(0x22);
	ssd1283a_set_cmd();

	ssd1283a_wait_txempty();
	ssd1283a_set_data();

	for (n = SSD1283A_MAX_X * SSD1283A_MAX_Y * SSD1283A_PIXEL_BYTES; n > 0; n--)
	{
		ssd1283a_write(0);
	}

	ssd1283a_wait_txempty();
	ssd1283a_unselectChip();
}

void ssd1283a_begin_fill()
{
	ssd1283a_selectChip();

	ssd1283a_write(0x22);
	ssd1283a_set_cmd();

	ssd1283a_wait_txempty();
	ssd1283a_set_data();
}

void ssd1283a_end_fill()
{
	ssd1283a_wait_txempty();
	ssd1283a_unselectChip();
}

void ssd1283a_init()
{
	volatile int i;

	ssd1283a_wr_reg(0x10, 0x2F, 0x8E); // Power control 1: DCY=Fline x 2, BTH=unregulated, DC=fosc, AP=Reserved
	ssd1283a_wr_reg(0x11, 0x00, 0x0C); // Power control 2: PU=x4
	ssd1283a_wr_reg(0x07, 0x00, 0x21);
	ssd1283a_wr_reg(0x28, 0x00, 0x06);
	ssd1283a_wr_reg(0x28, 0x00, 0x05);
	ssd1283a_wr_reg(0x27, 0x05, 0x7F);
	ssd1283a_wr_reg(0x29, 0x89, 0xA1);
	ssd1283a_wr_reg(0x00, 0x00, 0x01);
	for (i = 500000; i > 0; i--);

	ssd1283a_wr_reg(0x29, 0x80, 0xB0);
	for (i = 150000; i > 0; i--);

	ssd1283a_wr_reg(0x29, 0xFF, 0xFE);
	ssd1283a_wr_reg(0x07, 0x00, 0x21);
	for (i = 150000; i > 0; i--);

	ssd1283a_wr_reg(0x07, 0x00, 0x31);
	ssd1283a_wr_reg(0x01, 0x21, 0x83);
	ssd1283a_wr_reg(0x2F, 0xFF, 0xFF);
	ssd1283a_wr_reg(0x03, 0x68, 0x30);
	ssd1283a_wr_reg(0x40, 0x00, 0x02);
	ssd1283a_wr_reg(0x27, 0x05, 0x70);
	ssd1283a_wr_reg(0x02, 0x03, 0x00);
	ssd1283a_wr_reg(0x0B, 0x58, 0x0C);
	ssd1283a_wr_reg(0x12, 0x06, 0x09);
	ssd1283a_wr_reg(0x13, 0x31, 0x00);
	ssd1283a_wr_reg(0x2A, 0x1D, 0xD0);
	ssd1283a_wr_reg(0x2B, 0x0A, 0x90);
	ssd1283a_wr_reg(0x2D, 0x31, 0x0F);
	for (i = 500000; i > 0; i--);

	ssd1283a_wr_reg(0x1E, 0x00, 0xBF);
	for (i = 5000; i > 0; i--);

	ssd1283a_wr_reg(0x1E, 0x00, 0x00);
	for (i = 500000; i > 0; i--);

	ssd1283a_clear();

	ssd1283a_wr_reg(0x07, 0x00, 0x33);
}

/*-------------------------------------------------------------------*/
/* RTGUI driver implementation                                       */
/*-------------------------------------------------------------------*/

void rt_hw_ssd1283a_screen_update(rtgui_rect_t *rect)
{
	/* nothing for none-DMA mode driver */
}

rt_uint8_t *rt_hw_ssd1283a_get_framebuffer()
{
	return RT_NULL; /* no framebuffer driver */
}

void rt_hw_ssd1283a_set_pixel(rtgui_color_t *c, rt_base_t x, rt_base_t y)
{
	uint16_t p;

	/* get color pixel */
	p = rtgui_color_to_565p(*c);

	/* set x and y */
	ssd1283a_set_pos(x, y);
	ssd1283a_begin_fill();
	ssd1283a_fill_data(p >> 8, p);
	ssd1283a_end_fill();
}

void rt_hw_ssd1283a_get_pixel(rtgui_color_t *c, rt_base_t x, rt_base_t y)
{
	*c = rtgui_color_from_565p(0xffff);
}

void rt_hw_ssd1283a_draw_hline(rtgui_color_t *c, rt_base_t x1, rt_base_t x2, rt_base_t y)
{
	uint16_t p;
	uint8_t  ph;

	/* get color pixel */
	p = rtgui_color_to_565p(*c);
	ph = p >> 8;

	ssd1283a_set_pos(x1, y);
	ssd1283a_begin_fill();
	while (x1 < x2)
	{
		ssd1283a_fill_data(ph, p);
		x1++;
	}
	ssd1283a_end_fill();
}

void rt_hw_ssd1283a_draw_vline(rtgui_color_t *c, rt_base_t x, rt_base_t y1, rt_base_t y2)
{
	uint16_t p;
	uint8_t  ph;

	/* get color pixel */
	p = rtgui_color_to_565p(*c);
	ph = p >> 8;

	ssd1283a_set_rect(x, y1, x, y2);
	ssd1283a_begin_fill();
	while (y1 < y2)
	{
		ssd1283a_fill_data(ph, p);
		y1++;
	}
	ssd1283a_end_fill();
}

void rt_hw_ssd1283a_draw_raw_hline(rt_uint8_t *pixels, rt_base_t x1, rt_base_t x2, rt_base_t y)
{
	ssd1283a_set_pos(x1, y);
	ssd1283a_begin_fill();
	while (x1 < x2)
	{
		ssd1283a_fill_data(*pixels, *(pixels + 1));
		x1++;
		pixels += 2;
	}
	ssd1283a_end_fill();
}

static struct rtgui_graphic_driver _rtgui_ssd1283a_driver =
{
	"lcd",
	2,
	130,
	130,
	rt_hw_ssd1283a_screen_update,
	rt_hw_ssd1283a_get_framebuffer,
	rt_hw_ssd1283a_set_pixel,
	rt_hw_ssd1283a_get_pixel,
	rt_hw_ssd1283a_draw_hline,
	rt_hw_ssd1283a_draw_vline,
	rt_hw_ssd1283a_draw_raw_hline
};

void rt_hw_ssd1283a_init(void)
{
	ssd1283a_init();

	/* add lcd driver into graphic driver */
	rtgui_graphic_driver_add(&_rtgui_ssd1283a_driver);
}
