#include <stdio.h>
#include <stdlib.h>
#include <time.h>
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
    markov_build_probs(&chain);

    srand(time(NULL));

    // Generate a sequence starting from the first note in the training data
    uint8_t generated[32];
    uint8_t start = song.notes[0].pitch;

    markov_generate(&chain, start, generated, 32);

    printf("Generated sequence (starting from pitch %d):\n", start);
    for (int i = 0; i < 32; i++) {
        printf("%3d ", generated[i]);
        if ((i + 1) % 8 == 0) printf("\n");
    }

    return 0;
}
