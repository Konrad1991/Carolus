CC = cc
CFLAGS = -Wall -Wextra -std=c99

demo: main.c generic_array.c generic_array.h typed_array.h
	$(CC) $(CFLAGS) demo.c generic_array.c -o demo

clean:
	rm -f demo
