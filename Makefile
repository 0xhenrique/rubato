CC = gcc
CFLAGS = -Wall -g

rubato: src/main.c src/midi.c
	$(CC) $(CFLAGS) -o rubato src/main.c src/midi.c

clean:
	rm -f rubato
