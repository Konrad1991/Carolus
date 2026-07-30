#ifndef TEXTURES_H
#define TEXTURES_H

#include "types.h"

void init_texture_state(Texture_State* texture_state, const char* images_dir);

void free_texture_state(Texture_State* texture_state);

int figure_action_frame_count(FigureAction action);

#endif
