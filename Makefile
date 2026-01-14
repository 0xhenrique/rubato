CC = gcc
CFLAGS = -Wall -g

rubato: src/main.c src/midi.c src/markov.c
	$(CC) $(CFLAGS) -o rubato src/main.c src/midi.c src/markov.c

clean:
	rm -f rubato
