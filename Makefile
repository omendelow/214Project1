CC = gcc
CFLAGS = -g -std=c99 -Wvla -Wall -fsanitize=address,undefined

inputprocessor: inputprocessor.o
	$(CC) $(CFLAGS) -o $@ $^

inputprocessor.o: inputprocessor.c
	$(CC) -c $(CFLAGS) inputprocessor.c

ww: ww.o
	$(CC) $(CFLAGS) -o $@ $^

ww.o: ww.c
	$(CC) -c $(CFLAGS) ww.c

clean:
	rm -f *.o inputprocessor ww
