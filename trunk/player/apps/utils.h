/*
 * utils.h
 *
 *  Created on: 2010-9-14
 *      Author: Kyle
 */

#ifndef UTILS_H_
#define UTILS_H_

#define GBK2UNI_FILE "/resource/gbk2uni.tbl"
#define UNI2GBK_FILE "/resource/uni2gbk.tbl"

void ff_convert_init();
rt_uint16_t ff_convert(rt_uint16_t src, rt_uint32_t dir);
rt_uint16_t ff_wtoupper(rt_uint16_t chr);

#endif /* UTILS_H_ */
