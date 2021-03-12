CC = gcc
CFLAGS = -g -std=c99 -Wvla -Wall -fsanitize=address,undefined

ww: ww.o
	$(CC) $(CFLAGS) -o $@ $^

ww.o: ww.c
	$(CC) -c $(CFLAGS) ww.c

clean:
	rm -f *.o ww
