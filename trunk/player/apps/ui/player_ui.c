/*
 * player_ui.c
 *
 *  Created on: 2010-4-22
 *      Author: Kyle
 */

#include <rtgui/rtgui.h>
#include <rtgui/rtgui_system.h>
#include <rtgui/rtgui_server.h>
#include <rtgui/widgets/view.h>
#include <rtgui/widgets/list_view.h>
#include <rtgui/widgets/workbench.h>

#include "player_view.h"

static rt_thread_t player_ui_tid;
static rtgui_workbench_t *workbench;
static rtgui_view_t *menu_view;

extern rtgui_font_t rtgui_font_num16x19;

static rt_bool_t workbench_event_handler(rtgui_widget_t *widget, rtgui_event_t *event)
{
	if (event->type == RTGUI_EVENT_KBD)
	{
		struct rtgui_event_kbd *ekbd = (struct rtgui_event_kbd *) event;
		if (ekbd->type == RTGUI_KEYDOWN && ekbd->key == RTGUIK_MENU && !RTGUI_WORKBENCH_IS_MODAL_MODE(workbench))
		{
			/* active menu view */
			if (workbench->current_view != menu_view)
			{
				rtgui_view_show(menu_view, RT_FALSE);
				return RT_FALSE;
			}
		}
	}

	return rtgui_workbench_event_handler(widget, event);
}

static void player_ui_thread(void *p)
{
	rt_mq_t mq = rt_mq_create("ui", 256, 4, RT_IPC_FLAG_FIFO);
	rtgui_thread_register(rt_thread_self(), mq);

	static const rtgui_rect_t rect =
	{
		.x1 = 0,
		.y1 = 0,
		.x2 = 130,
		.y2 = 130
	};

	rtgui_font_system_add_font(&rtgui_font_num16x19);

	rtgui_panel_register("main", (rtgui_rect_t *) &rect);
	rtgui_panel_set_default_focused("main");

	workbench = rtgui_workbench_create("main", (const unsigned char *) "workbench");
	rtgui_widget_set_event_handler(RTGUI_WIDGET(workbench), workbench_event_handler);

	player_view_create(workbench);
//	menu_view_create(workbench);

	rtgui_workbench_event_loop(workbench);

	rtgui_thread_deregister(rt_thread_self());
	rt_mq_delete(mq);
}

void player_ui_init()
{
	player_ui_tid = rt_thread_create("player", player_ui_thread, RT_NULL, 2048, 20, 5);
	rt_thread_startup(player_ui_tid);
}
