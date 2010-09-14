/*
 * startup.c
 *
 *  Created on: 2010-4-22
 *      Author: Kyle
 *
 * $Id$
 */

#include <compiler.h>
#include <board.h>

#include <rtthread.h>
#include <finsh.h>
#include <dfs_init.h>
#include <dfs_elm.h>
#include <dfs_fs.h>
#include <rtgui/rtgui_system.h>

#include "../drivers/console/usart_console.h"
#include "../drivers/lcd/ssd1283a.h"
#include "../drivers/sd_mmc/sd_mmc.h"
#include "utils.h"

#include "ui/player_ui.h"

void player_init_thread(void *p)
{
	rt_show_version();

	dfs_init();
	elm_init();

//	if (dfs_mount("spi0", "/", "elm", 0, 0) != 0)
//		rt_kprintf("Unable to mount spi0 as /.\n");

	if (dfs_mount("sd0", "/", "elm", 0, 0) != 0)
		rt_kprintf("Unable to mount sd0 as /.\n");

	ff_convert_init();
	rtgui_system_server_init();
	player_ui_init();
}

int main()
{
#ifdef RT_USING_HEAP
	extern void __heap_start__;
	extern void __heap_end__;
#endif

	rt_hw_board_init();

	rt_system_tick_init();
	rt_system_object_init();
	rt_system_timer_init();

#ifdef RT_USING_HEAP
	rt_system_heap_init(&__heap_start__, &__heap_end__);
#endif

	rt_system_scheduler_init();

	rt_hw_ssd1283a_init();
//	rt_hw_i2s_init();
//	rt_hw_at26df321_init();
	rt_hw_sd_mmc_init();

	rt_device_init_all();

#ifdef RT_USING_FINSH
    /* init finsh */
	extern void finsh_system_init();
    finsh_system_init();
    finsh_set_device(FINSH_DEVICE_NAME);
#endif

    rt_thread_idle_init();

	rt_thread_t tinit = rt_thread_create("init", player_init_thread, RT_NULL, 1024, 0, 10);
	rt_thread_startup(tinit);

	rt_system_scheduler_start();

	return 0;
}
