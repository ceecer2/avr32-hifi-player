/*
 * player_view.h
 *
 *  Created on: 2010-4-25
 *      Author: Kyle
 */

#ifndef PLAYER_VIEW_H_
#define PLAYER_VIEW_H_

#define PLAYER_REGION_HEADER		0
#define PLAYER_REGION_ALBUM			1
#define PLAYER_REGION_TITLE			2
#define PLAYER_REGION_ARTIST		3
#define PLAYER_REGION_ELAPSED_TIME	4
#define PLAYER_REGION_PROGRESS_BAR	5
#define PLAYER_REGION_ALBUM_INFO	6
#define PLAYER_REGION_PLAYER_INFO	7

extern rtgui_view_t *player_view;

void player_view_create(rtgui_workbench_t *workbench);

#endif /* PLAYER_VIEW_H_ */
