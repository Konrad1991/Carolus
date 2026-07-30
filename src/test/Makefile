CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -I..
LDLIBS = -lraylib -lX11 -lm

SRCS = test_main.c test_soil.c \
       ../map/map.c ../a_star/a_star.c ../flood_fill/flood_fill.c \
       ../soil/soil.c ../utils/game_time.c ../utils/utils.c
TARGET = run_tests

$(TARGET): $(SRCS) test.h
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET) $(LDLIBS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
