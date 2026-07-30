CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -I. -MMD -MP
LDLIBS = -lraylib -lX11 -lm

SRCS = main.c \
			 utils/utils.c utils/game_time.c \
			 mode/mode.c \
			 map/map.c map/map_setup.c \
			 a_star/a_star.c flood_fill/flood_fill.c drawing/drawing.c drawing/drawing_helper.c \
       drawing/update_tile_state.c drawing/update_figure.c drawing/update_grass.c \
			 drawing/update_tree.c drawing/update_wheat.c drawing/build_object.c \
       drawing/build_road.c \
       textures/textures.c sidebar/sidebar.c selection/selection.c units/units.c \
       containers/arrays.c weather/weather.c weather/weather_scenario.c weather/clouds.c weather/puddles.c \
       soil/soil.c soil/soil_overlay.c \
       farming/crop_growth.c \
			 seasons/seasons.c \
       minimap/minimap.c
OBJS = $(SRCS:.c=.o)
DEPS = $(OBJS:.o=.d)
TARGET = main

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET) $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

-include $(DEPS)

clean:
	rm -f $(TARGET) $(OBJS) $(DEPS)

.PHONY: clean
