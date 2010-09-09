/*
 * player_view.c
 *
 *  Created on: 2010-4-25
 *      Author: Kyle
 */

#include <rtgui/rtgui_system.h>
#include <rtgui/dc_hw.h>
#include <rtgui/image.h>
#include <rtgui/font.h>

#include <rtgui/widgets/progressbar.h>
#include <rtgui/widgets/workbench.h>

#include "player_view.h"
#include "lang_zh_CN.h"

rtgui_view_t *player_view;
struct rtgui_progressbar *progress_bar;

static const rtgui_rect_t TEXT_REGIONS[] =
{
	{4,  0,  126,  18},	// PLAYER_REGION_HEADER
	{4,  18, 126,  34},	// PLAYER_REGION_ALBUM
	{4,  34, 126,  50},	// PLAYER_REGION_TITLE
	{4,  50, 126,  66},	// PLAYER_REGION_ARTIST
	{4,  66, 126,  90},	// PLAYER_REGION_ELAPSED_TIME
	{8,  90, 122,  98},	// PLAYER_REGION_PROGRESS_BAR
	{4,  98, 126, 114},	// PLAYER_REGION_ALBUM_INFO
	{4, 114, 126, 130},	// PLAYER_REGION_PLAYER_INFO
};

extern rtgui_font_t rtgui_font_num16x19;

static rt_bool_t paint_handler(rtgui_widget_t *widget, rtgui_event_t *event)
{
	struct rtgui_dc *dc = rtgui_dc_begin_drawing(widget);

	RTGUI_DC_FC(dc) = RTGUI_RGB(255, 255, 255);
	RTGUI_DC_TEXTALIGN(dc) = RTGUI_ALIGN_CENTER_HORIZONTAL | RTGUI_ALIGN_CENTER_VERTICAL;

	rtgui_dc_draw_text(dc, LANG_PLAYER_HEADER_DEFAULT, (rtgui_rect_t *) &TEXT_REGIONS[PLAYER_REGION_HEADER]);
	rtgui_dc_draw_text(dc, LANG_PLAYER_ALBUM_UNKNOWN, (rtgui_rect_t *) &TEXT_REGIONS[PLAYER_REGION_ALBUM]);
	rtgui_dc_draw_text(dc, LANG_PLAYER_TITLE_UNKNOWN, (rtgui_rect_t *) &TEXT_REGIONS[PLAYER_REGION_TITLE]);
	rtgui_dc_draw_text(dc, LANG_PLAYER_ARTIST_UNKNOWN, (rtgui_rect_t *) &TEXT_REGIONS[PLAYER_REGION_ARTIST]);

	RTGUI_DC_FONT(dc) = &rtgui_font_num16x19;
	rtgui_dc_draw_text(dc, "0:00:00", (rtgui_rect_t *) &TEXT_REGIONS[PLAYER_REGION_ELAPSED_TIME]);

	RTGUI_DC_FONT(dc) = rtgui_font_default();
	RTGUI_DC_TEXTALIGN(dc) = RTGUI_ALIGN_RIGHT | RTGUI_ALIGN_CENTER_VERTICAL;
	rtgui_dc_draw_text(dc, "0/0", (rtgui_rect_t *) &TEXT_REGIONS[PLAYER_REGION_ALBUM_INFO]);

	RTGUI_DC_TEXTALIGN(dc) = RTGUI_ALIGN_LEFT | RTGUI_ALIGN_CENTER_VERTICAL;
	rtgui_dc_draw_text(dc, "0:00:00", (rtgui_rect_t *) &TEXT_REGIONS[PLAYER_REGION_ALBUM_INFO]);

	rtgui_dc_draw_text(dc, "24bit/44.1k 2CH WAVE", (rtgui_rect_t *) &TEXT_REGIONS[PLAYER_REGION_PLAYER_INFO]);

	rtgui_dc_end_drawing(dc);

	return RT_FALSE;
}

static rt_bool_t key_handler(rtgui_widget_t *widget, rtgui_event_t *event)
{
	return RT_FALSE;
}

static rt_bool_t command_handler(rtgui_widget_t *widget, rtgui_event_t *event)
{
	return RT_FALSE;
}

static rt_bool_t player_view_event_handler(rtgui_widget_t *widget, rtgui_event_t *event)
{
	switch (event->type)
	{
	case RTGUI_EVENT_PAINT:
		return paint_handler(widget, event);

	case RTGUI_EVENT_KBD:
		return key_handler(widget, event);

	case RTGUI_EVENT_COMMAND:
		return command_handler(widget, event);

	default:
		return rtgui_view_event_handler(widget, event);
	}
}

void player_view_create(rtgui_workbench_t *workbench)
{
	player_view = rtgui_view_create("player");
	rtgui_widget_set_event_handler(RTGUI_WIDGET(player_view), player_view_event_handler);

	rtgui_workbench_add_view(workbench, player_view);
	RTGUI_WIDGET(player_view)->flag |= RTGUI_WIDGET_FLAG_FOCUSABLE;
	rtgui_widget_focus(RTGUI_WIDGET(player_view));
	rtgui_view_show(player_view, RT_FALSE);
}
