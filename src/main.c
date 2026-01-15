#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "midi.h"
#include "markov.h"
#include "mood.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <midi_file> [--mood sad|happy|energetic]\n", argv[0]);
        return 1;
    }

    Mood mood = MOOD_NEUTRAL;
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "--mood") == 0 && i + 1 < argc) {
            mood = mood_parse(argv[i + 1]);
            i++;
        }
    }

    Song song;
    if (midi_parse(argv[1], &song) != 0) {
        printf("Failed to parse: %s\n", argv[1]);
        return 1;
    }

    printf("Parsed %d notes\n", song.count);
    if (mood != MOOD_NEUTRAL) {
        const char *mood_names[] = {"neutral", "sad", "happy", "energetic"};
        printf("Applying mood filter: %s\n", mood_names[mood]);
    }
    printf("\n");

    MarkovChain chain;
    markov_init(&chain);
    markov_train(&chain, &song);
    markov_build_probs(&chain);
    mood_apply(&chain, mood);

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

    // Write to MIDI file
    const char *output_file = "output/generated.mid";
    if (midi_write(output_file, generated, 32) == 0) {
        printf("\nWritten to %s\n", output_file);
    } else {
        printf("\nFailed to write %s\n", output_file);
    }

    return 0;
}
