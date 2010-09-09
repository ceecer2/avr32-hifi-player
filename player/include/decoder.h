/*
 * decoder.h
 *
 *  Created on: 2010-4-29
 *      Author: Kyle
 */

#ifndef DECODER_H_
#define DECODER_H_

#include <stdint.h>

struct decoder_info
{
	const char *file_name;
	int fd;
	uint32_t file_size;
	uint32_t file_position;

	uint32_t duration;

	uint8_t channels;
	uint32_t sample_rate;
	uint32_t bit_rate;
	uint32_t bits_per_sample;
};
typedef struct decoder_info* decoder_info_t;

struct decoder
{
	decoder_info_t (*open)(const char *filename);
	void (*close)(decoder_info_t info);
	uint32_t (*get_next_block)(decoder_info_t info, uint8_t *buf, uint32_t size);
	uint32_t (*seek)(decoder_info_t info, uint32_t time);
};
typedef struct decoder* decoder_t;

#endif /* DECODER_H_ */
