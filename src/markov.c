#include "markov.h"
#include <stdio.h>
#include <string.h>

void markov_init(MarkovChain *chain) {
    memset(chain->counts, 0, sizeof(chain->counts));
    chain->total_transitions = 0;
}

void markov_train(MarkovChain *chain, Song *song) {
    // Iterate notes in order, count transitions
    for (int i = 0; i < song->count - 1; i++) {
        uint8_t from = song->notes[i].pitch;
        uint8_t to = song->notes[i + 1].pitch;
        chain->counts[from][to]++;
        chain->total_transitions++;
    }
}

// Convert MIDI pitch to note name
static const char* pitch_name(int pitch) {
    static char buf[8];
    const char *names[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    int octave = (pitch / 12) - 1;
    int note = pitch % 12;
    sprintf(buf, "%s%d", names[note], octave);
    return buf;
}

void markov_print_top(MarkovChain *chain, int n) {
    printf("Top %d transitions (out of %d total):\n", n, chain->total_transitions);

    // Find max N times
    for (int rank = 0; rank < n; rank++) {
        int max_count = 0;
        int max_from = 0, max_to = 0;

        for (int from = 0; from < NUM_PITCHES; from++) {
            for (int to = 0; to < NUM_PITCHES; to++) {
                if (chain->counts[from][to] > max_count) {
                    max_count = chain->counts[from][to];
                    max_from = from;
                    max_to = to;
                }
            }
        }

        if (max_count == 0) break;

        printf("  %s -> %s : %d times\n",
               pitch_name(max_from), pitch_name(max_to), max_count);

        // Zero it out in order to find the next highest
        chain->counts[max_from][max_to] = 0;
    }
}
