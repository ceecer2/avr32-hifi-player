/*
 * wav.c
 *
 *  Created on: 2010-4-29
 *      Author: Kyle
 */

#include <rtthread.h>
#include <dfs_posix.h>
#include <finsh.h>
#include <stdint.h>

#include "decoder.h"

#define RIFF_CHUNK_ID		0x52494646	// "RIFF"
#define FMT_CHUNK_ID		0x666D7420	// "fmt "
#define DATA_CHUNK_ID		0x64617461	// "data"
#define FORMAT_WAVE			0x57415645	// "WAVE"
#define AUDIO_FORMAT_PCM	1
#define HEADER_SIZE			(sizeof(struct riff_chunk) + sizeof(struct fmt_chunk) + sizeof(struct data_chunk))

struct riff_chunk
{
	uint32_t chunk_id;
	uint32_t chunk_size;
	uint32_t format;
};

struct fmt_chunk
{
	uint32_t chunk_id;
	uint32_t chunk_size;
	uint16_t audio_format;
	uint16_t num_channels;
	uint32_t sample_rate;
	uint32_t byte_rate;
	uint16_t block_align;
	uint16_t bits_per_sample;
};

struct data_chunk
{
	uint32_t chunk_id;
	uint32_t chunk_size;
};

static decoder_info_t decoder_open(const char *filename)
{
	decoder_info_t info = malloc(sizeof(struct decoder_info));
	struct riff_chunk riff;
	struct fmt_chunk fmt;
	struct data_chunk data;

	do
	{
		info->fd = open(filename, O_RDONLY, 0);
		if (info->fd < 0)
			break;

		info->file_name = filename;

		read(info->fd, (char *) &riff, sizeof(riff));
		if (riff.chunk_id != RIFF_CHUNK_ID || riff.format != FORMAT_WAVE)
			break;

		info->file_size = riff.chunk_size + 8;

		read(info->fd, (char *) &fmt, sizeof(fmt));
		if (fmt.chunk_id != FMT_CHUNK_ID && fmt.audio_format != AUDIO_FORMAT_PCM)
			break;

		info->channels = fmt.num_channels;
		info->sample_rate = fmt.sample_rate;
		info->bit_rate = fmt.byte_rate * 8;
		info->bits_per_sample = fmt.bits_per_sample;

		read(info->fd, (char *) &data, sizeof(data));
		if (data.chunk_id != DATA_CHUNK_ID)
			break;

		info->duration = data.chunk_size / fmt.byte_rate;
		info->file_position = HEADER_SIZE;

		return info;
	} while (0);

	// Error occurred
	free(info);
	return NULL;
}

static void decoder_close(decoder_info_t info)
{
	close(info->fd);
	free(info);
}

static uint32_t decoder_get_next_block(decoder_info_t info, uint8_t *buf, uint32_t size)
{
	int c = read(info->fd, (char *) buf, size);
	if (c > 0)
	{
		info->file_position += c;
	}
}

static uint32_t decoder_seek(decoder_info_t info, uint32_t time)
{
	uint32_t pos = time * info->bit_rate * 8;
	if (pos + HEADER_SIZE > info->file_size)
		return info->file_position - (HEADER_SIZE);

	info->file_position = pos + HEADER_SIZE;
	lseek(info->fd, info->file_position, SEEK_SET);

	return pos;
}

decoder_t wav_decoder =
{
	decoder_open,
	decoder_close,
	decoder_get_next_block,
	decoder_seek
};
