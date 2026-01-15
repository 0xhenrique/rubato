#include "mood.h"
#include <string.h>
#include <stdlib.h>

Mood mood_parse(const char *str) {
    if (strcmp(str, "sad") == 0) return MOOD_SAD;
    if (strcmp(str, "happy") == 0) return MOOD_HAPPY;
    if (strcmp(str, "energetic") == 0) return MOOD_ENERGETIC;
    return MOOD_NEUTRAL;
}

// Get multiplier for a given interval (in semitones) based on mood
static float interval_multiplier(int interval, Mood mood) {
    // Normalize interval to 0-11 range (ignore octave)
    int normalized = abs(interval) % 12;

	// @TODO: This is still a WIP
    switch (mood) {
        case MOOD_SAD:
            // Favor minor intervals: minor 3rd (3), minor 6th (8), minor 7th (10)
            if (normalized == 3 || normalized == 8 || normalized == 10) {
                return 2.0f;
            }
            // Reduce major intervals
            if (normalized == 4 || normalized == 9) {
                return 0.5f;
            }
            break;

        case MOOD_HAPPY:
            // Favor major intervals: major 3rd (4), perfect 5th (7), major 6th (9)
            if (normalized == 4 || normalized == 7 || normalized == 9) {
                return 2.0f;
            }
            // Reduce minor intervals
            if (normalized == 3 || normalized == 8 || normalized == 10) {
                return 0.5f;
            }
            break;

        case MOOD_ENERGETIC:
            // Favor larger jumps (> 5 semitones)
            if (abs(interval) > 5) {
                return 1.5f;
            }
            // Reduce small steps
            if (abs(interval) <= 2) {
                return 0.7f;
            }
            break;

        case MOOD_NEUTRAL:
        default:
            break;
    }

    return 1.0f;
}

// Get multiplier based on target pitch range
static float pitch_multiplier(int pitch, Mood mood) {
    switch (mood) {
        case MOOD_SAD:
            // Favor lower octaves
            if (pitch < 60) return 1.3f;
            if (pitch >= 72) return 0.7f;
            break;

        case MOOD_HAPPY:
            // Favor higher octaves
            if (pitch >= 60) return 1.3f;
            if (pitch < 48) return 0.7f;
            break;

        case MOOD_ENERGETIC:
            // Favor larger ranges
            if (pitch < 48 || pitch >= 84) return 1.2f;
            break;

        case MOOD_NEUTRAL:
        default:
            break;
    }

    return 1.0f;
}

void mood_apply(MarkovChain *chain, Mood mood) {
    if (mood == MOOD_NEUTRAL) return;

    for (int from = 0; from < NUM_PITCHES; from++) {
        float row_sum = 0;

        // First pass: apply multipliers
        for (int to = 0; to < NUM_PITCHES; to++) {
            if (chain->probs[from][to] > 0) {
                int interval = to - from;
                float mult = interval_multiplier(interval, mood) * pitch_multiplier(to, mood);
                chain->probs[from][to] *= mult;
            }
            row_sum += chain->probs[from][to];
        }

        // Second pass: normalize in order to row sums to 1
        if (row_sum > 0) {
            for (int to = 0; to < NUM_PITCHES; to++) {
                chain->probs[from][to] /= row_sum;
            }
        }
    }
}
