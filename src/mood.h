#ifndef MOOD_H
#define MOOD_H

#include "markov.h"

typedef enum {
    MOOD_NEUTRAL,
    MOOD_SAD,
    MOOD_HAPPY,
    MOOD_ENERGETIC
} Mood;

Mood mood_parse(const char *str);
void mood_apply(MarkovChain *chain, Mood mood);

#endif
