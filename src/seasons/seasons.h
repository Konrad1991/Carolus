#ifndef SEASONS_H
#define SEASONS_H

#include "types.h"

SeasonBlend resolve_season_blend(SeasonState s);
void update_season_state(SeasonState* s);
void draw_season_bar(SeasonState *s);
Rectangle topbar_panel_rect(void);


#endif
