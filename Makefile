CC = gcc
CFLAGS = -Wall -g

rubato: src/main.c src/midi.c src/markov.c src/mood.c
	$(CC) $(CFLAGS) -o rubato src/main.c src/midi.c src/markov.c src/mood.c

clean:
	rm -f rubato
