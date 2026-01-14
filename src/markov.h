#ifndef MARKOV_H
#define MARKOV_H

#include "midi.h"

// 128 possible MIDI pitches (0-127)
#define NUM_PITCHES 128

typedef struct {
    int counts[NUM_PITCHES][NUM_PITCHES];   // counts[from][to]
    float probs[NUM_PITCHES][NUM_PITCHES];  // probabilities (each row sums to 1)
    int total_transitions;
} MarkovChain;

void markov_init(MarkovChain *chain);
void markov_train(MarkovChain *chain, Song *song);
void markov_build_probs(MarkovChain *chain);
void markov_generate(MarkovChain *chain, uint8_t start_pitch, uint8_t *output, int length);
void markov_print_top(MarkovChain *chain, int n);

#endif
