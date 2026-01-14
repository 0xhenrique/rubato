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
    markov_build_probs(&chain);

    // @TODO: this is just an example to show probabilities
    int example_pitch = 67;
    printf("After G4, what comes next?\n");
    for (int to = 0; to < NUM_PITCHES; to++) {
        if (chain.probs[example_pitch][to] > 0.01) { // only show >1%
            printf("  -> pitch %3d : %.1f%%\n", to, chain.probs[example_pitch][to] * 100);
        }
    }

    return 0;
}
