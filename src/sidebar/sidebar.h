#ifndef SIDEBAR_H
#define SIDEBAR_H

#include "raylib.h"
#include "raymath.h"
#include "types.h"
#include <stdio.h>

void init_sidebar_state(SidebarState* sidebar_state, const Texture_State *texture_state,
                        const int SIDEBAR_WIDTH, const int SIDEBAR_PADDING,
                        const int FONT_SIZE, const float ICON_WIDTH, const float ICON_HEIGHT);
void update_sidebar_state(SidebarState* sidebar_state);
bool draw_icon(SidebarState sidebar_state, Texture2D tex, float si_h, int x, int y);
void draw_sidebar(SidebarState* sidebar_state, const Texture_State *texture_state, bool hovered[MODE_COUNT]);

#endif
