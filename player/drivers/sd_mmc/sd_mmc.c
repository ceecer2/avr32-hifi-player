/*
 * sd_mmc.c
 *
 *  Created on: 2010-4-25
 *      Author: Kyle
 */

#include <compiler.h>
#include <gpio.h>
#include <pdca.h>
#include <sd_mmc_spi.h>

#include <dfs_fs.h>

static struct dfs_partition sd_mmc_partition;
static const char dummy[MMC_SECTOR_SIZE] =
{
#define DUMMY_8_BYTES(line, unsed)	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	MREPEAT(64, DUMMY_8_BYTES, ~)
#undef DUMMY_8_BYTES
};

volatile avr32_pdca_channel_t *pdca_channel_rx;
volatile avr32_pdca_channel_t *pdca_channel_tx;

static rt_err_t sd_mmc_init(rt_device_t dev)
{
	pdca_channel_rx = pdca_get_handler(PDCA_CHANNEL_SPI_RX);
	pdca_channel_tx = pdca_get_handler(PDCA_CHANNEL_SPI_TX);

	return RT_EOK;
}

static rt_err_t sd_mmc_open(rt_device_t dev, rt_uint16_t oflag)
{
	return RT_EOK;
}

static rt_err_t sd_mmc_close(rt_device_t dev)
{
	return RT_EOK;
}

static rt_size_t sd_mmc_read(rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size)
{
	rt_uint32_t count = size;
	rt_uint32_t sector = sd_mmc_partition.offset + pos;

	while (count)
	{
		if (sd_mmc_spi_read_open_PDCA(sector))
		{
			pdca_load_channel(PDCA_CHANNEL_SPI_RX, buffer, MMC_SECTOR_SIZE);
			pdca_load_channel(PDCA_CHANNEL_SPI_TX, (void *) dummy, MMC_SECTOR_SIZE);

			spi_write(SD_MMC_SPI, 0xFF);

			pdca_channel_rx->cr = AVR32_PDCA_TEN_MASK;
			pdca_channel_tx->cr = AVR32_PDCA_TEN_MASK;

			while (!(pdca_channel_rx->isr & AVR32_PDCA_ISR_TRC_MASK));

			sd_mmc_spi_read_close_PDCA();

			pdca_channel_rx->cr = AVR32_PDCA_TDIS_MASK;
			pdca_channel_tx->cr = AVR32_PDCA_TDIS_MASK;

			count--;
			buffer += MMC_SECTOR_SIZE;
		}
		else
		{
			rt_kprintf("Unable to initiate SD/MMC read command.\n");
			return 0;
		}
	}

	return size;
}

static rt_size_t sd_mmc_write(rt_device_t dev, rt_off_t pos, const void* buffer, rt_size_t size)
{
	rt_uint32_t count = size;
	rt_uint32_t sector = sd_mmc_partition.offset + pos;

	if (sd_mmc_spi_write_open(sector))
	{
		while (count)
		{
			if (sd_mmc_spi_write_sector_from_ram(buffer))
			{
				count--;
				buffer += MMC_SECTOR_SIZE;
			}
			else
			{
				rt_kprintf("SD/MMC write error.\n");
				size = 0;
				break;
			}
		}
		sd_mmc_spi_write_close();
	}
	else
	{
		rt_kprintf("Unable to initiate SD/MMC write command.\n");
		return 0;
	}

	return size;
}

static rt_err_t sd_mmc_control(rt_device_t dev, rt_uint8_t cmd, void *args)
{
	return RT_EOK;
}

static struct rt_device sd_mmc_device =
{
	.type		= RT_Device_Class_Block,
	.init		= sd_mmc_init,
	.open		= sd_mmc_open,
	.close		= sd_mmc_close,
	.read		= sd_mmc_read,
	.write	 	= sd_mmc_write,
	.control	= sd_mmc_control
};

void rt_hw_sd_mmc_init()
{
	if (sd_mmc_spi_internal_init())
	{
		rt_uint8_t *sector = rt_malloc(MMC_SECTOR_SIZE);
		sd_mmc_spi_read_open(0);
		if (sd_mmc_spi_read_sector_to_ram(sector))
		{
			if (dfs_filesystem_get_partition(&sd_mmc_partition, sector, 0) != RT_EOK)
			{
				sd_mmc_partition.offset = 0;
				sd_mmc_partition.size = 0;
			}
		}
		else
		{
			sd_mmc_partition.offset = 0;
			sd_mmc_partition.size = 0;
		}
		sd_mmc_spi_read_close();
		rt_free(sector);

		rt_device_register(&sd_mmc_device, "sd0", RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_REMOVABLE | RT_DEVICE_FLAG_STANDALONE);
	}
	else
	{
		rt_kprintf("Failed to initialize SD/MMC card.\n");
	}
}
