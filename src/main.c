#include <stdio.h>
#include "midi.h"
#include "markov.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <midi_file>\n", argv[0]);
        return 1;
    }

    Song song;
    if (midi_parse(argv[1], &song) != 0) {
        printf("Failed to parse: %s\n", argv[1]);
        return 1;
    }

    printf("Parsed %d notes\n\n", song.count);

    MarkovChain chain;
    markov_init(&chain);
    markov_train(&chain, &song);
    markov_print_top(&chain, 15);

    return 0;
}
