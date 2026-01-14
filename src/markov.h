#ifndef MARKOV_H
#define MARKOV_H

#include "midi.h"

// 128 possible MIDI pitches (0-127)
#define NUM_PITCHES 128

typedef struct {
    int counts[NUM_PITCHES][NUM_PITCHES]; // counts[from][to]
    int total_transitions;
} MarkovChain;

void markov_init(MarkovChain *chain);
void markov_train(MarkovChain *chain, Song *song);
void markov_print_top(MarkovChain *chain, int n);

#endif
