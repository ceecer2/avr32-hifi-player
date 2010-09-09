/*
 * usart.c
 *
 *  Created on: 2010-4-23
 *      Author: Kyle
 */

#include "usart_console.h"

static rt_err_t usart_init(rt_device_t device);
static rt_err_t usart_open(rt_device_t device, rt_uint16_t oflag);
static rt_err_t usart_close(rt_device_t device);
static rt_size_t usart_read(rt_device_t device, rt_off_t pos, void *buffer, rt_size_t size);
static rt_size_t usart_write(rt_device_t device, rt_off_t pos, const void *buffer, rt_size_t size);
static rt_err_t usart_control(rt_device_t device, rt_uint8_t cmd, void *args);

static struct rt_device device =
{
	.type			= RT_Device_Class_Char,
	.rx_indicate	= RT_NULL,
	.tx_complete	= RT_NULL,
	.init 			= usart_init,
	.open			= usart_open,
	.close			= usart_close,
	.read 			= usart_read,
	.write 			= usart_write,
	.control 		= usart_control,
	.private		= (void *) CONSOLE_PORT
};

static const usart_options_t USART_OPTIONS = {
	.baudrate = 115200,
	.charlength = 8,
	.paritytype = USART_NO_PARITY,
	.stopbits = USART_1_STOPBIT,
	.channelmode = USART_NORMAL_CHMODE
};

static rt_uint8_t rx_buffer[RX_BUFFER_SIZE];
static rt_uint32_t rx_read_pos, rx_write_pos;

static void usart_rx_int_handler()
{
	if (usart_test_hit(CONSOLE_PORT))
	{
		rt_base_t level = rt_hw_interrupt_disable();

		rx_buffer[rx_write_pos] = CONSOLE_PORT->rhr;
		rx_write_pos++;
		if (rx_write_pos >= RX_BUFFER_SIZE)
		{
			rx_write_pos = 0;
		}

		// Check for buffer overflow
		if (rx_read_pos == rx_write_pos)
		{
			rx_read_pos++;
			if (rx_read_pos >= RX_BUFFER_SIZE)
			{
				rx_read_pos = 0;
			}
		}

		rt_hw_interrupt_enable(level);

		if (device.rx_indicate != RT_NULL)
		{
			rt_size_t size = rx_read_pos > rx_write_pos ?
					RX_BUFFER_SIZE - rx_read_pos + rx_write_pos :
					rx_write_pos - rx_read_pos;

			device.rx_indicate(&device, size);
		}
	}
	else
	{
		usart_reset_status(CONSOLE_PORT);
	}
}

static rt_err_t usart_init(rt_device_t dev)
{
	if (!(dev->flag & RT_DEVICE_FLAG_ACTIVATED))
	{
		gpio_enable_module_pin(CONSOLE_PORT_TXD_PIN, CONSOLE_PORT_TXD_FUNCTION);
		gpio_enable_module_pin(CONSOLE_PORT_RXD_PIN, CONSOLE_PORT_RXD_FUNCTION);
		usart_init_rs232(CONSOLE_PORT, &USART_OPTIONS, gPBAFreq);

		INTC_register_interrupt(&usart_rx_int_handler, CONSOLE_PORT_IRQ, CONSOLE_PORT_IRQ_LEVEL);
		CONSOLE_PORT->ier = AVR32_USART_IER_RXRDY_MASK;

		rt_memset(rx_buffer, 0, RX_BUFFER_SIZE);
		rx_read_pos = 0;
		rx_write_pos = 0;

		dev->flag |= RT_DEVICE_FLAG_ACTIVATED;
	}
	return RT_EOK;
}

static rt_err_t usart_open(rt_device_t dev, rt_uint16_t oflag)
{
	return RT_EOK;
}

static rt_err_t usart_close(rt_device_t dev)
{
	return RT_EOK;
}

static rt_size_t usart_read(rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size)
{
	rt_uint8_t *ptr = buffer;
	rt_err_t err_code = RT_EOK;

	rt_base_t level = rt_hw_interrupt_disable();

	rt_size_t rx_count = rx_read_pos > rx_write_pos ?
			RX_BUFFER_SIZE - rx_read_pos + rx_write_pos :
			rx_write_pos - rx_read_pos;

	if (rx_count < size)
	{
		size = rx_count;
		err_code = -RT_EEMPTY;
	}
	else
	{
		rx_count = size;
	}

	while (size)
	{
		*ptr++ = rx_buffer[rx_read_pos++];
		if (rx_read_pos >= RX_BUFFER_SIZE)
		{
			rx_read_pos = 0;
		}
		size--;
	}

	rt_hw_interrupt_enable(level);

	rt_set_errno(err_code);
	return rx_count;
}

static rt_size_t usart_write(rt_device_t dev, rt_off_t pos, const void *buffer, rt_size_t size)
{
	rt_uint8_t *ptr = (rt_uint8_t *) buffer;
	rt_err_t err_code = RT_EOK;

	do
	{
		if (usart_putchar(CONSOLE_PORT, *ptr++) != USART_SUCCESS)
		{
			err_code = -RT_EBUSY;
			break;
		}
	} while (--size);

	rt_set_errno(err_code);
	return (rt_size_t) ptr - (rt_size_t) buffer;
}

static rt_err_t usart_control(rt_device_t dev, rt_uint8_t cmd, void *args)
{
	if (cmd == RT_DEVICE_CTRL_SYSCLOCK_CHANGED)
	{
		usart_init_rs232(CONSOLE_PORT, &USART_OPTIONS, gPBAFreq);
	}
	return RT_EOK;
}

void usart_console_init()
{
	rt_device_register(&device, CONSOLE_DEVICE_NAME, RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_STREAM | RT_DEVICE_FLAG_INT_RX);
	rt_console_set_device(CONSOLE_DEVICE_NAME);
}
