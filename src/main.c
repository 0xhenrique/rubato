#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include "midi.h"
#include "markov.h"
#include "mood.h"

static int ends_with(const char *str, const char *suffix) {
    size_t str_len = strlen(str);
    size_t suffix_len = strlen(suffix);
    if (suffix_len > str_len) return 0;
    return strcmp(str + str_len - suffix_len, suffix) == 0;
}

static int train_from_file(const char *filepath, MarkovChain *chain, uint8_t *first_pitch) {
    Song song;
    if (midi_parse(filepath, &song) != 0) {
        printf("  [SKIP] Failed to parse: %s\n", filepath);
        return 0;
    }

    printf("  [OK] %s (%d notes)\n", filepath, song.count);
    markov_train(chain, &song);

    // Save first pitch from first successful file
    if (*first_pitch == 0 && song.count > 0) {
        *first_pitch = song.notes[0].pitch;
    }

    return song.count;
}

static int train_from_directory(const char *dirpath, MarkovChain *chain, uint8_t *first_pitch) {
    DIR *dir = opendir(dirpath);
    if (!dir) {
        printf("Failed to open directory: %s\n", dirpath);
        return -1;
    }

    int total_notes = 0;
    int file_count = 0;
    struct dirent *entry;

    printf("Training from directory: %s\n", dirpath);

    while ((entry = readdir(dir)) != NULL) {
        if (!ends_with(entry->d_name, ".mid") && !ends_with(entry->d_name, ".midi")) {
            continue;
        }

        // Build full path
        char filepath[512];
        snprintf(filepath, sizeof(filepath), "%s/%s", dirpath, entry->d_name);

        int notes = train_from_file(filepath, chain, first_pitch);
        if (notes > 0) {
            total_notes += notes;
            file_count++;
        }
    }

    closedir(dir);

    printf("\nTrained on %d files, %d total notes\n", file_count, total_notes);
    return total_notes;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <midi_file> [--mood sad|happy|energetic]\n", argv[0]);
        printf("       %s --dir <directory> [--mood sad|happy|energetic]\n", argv[0]);
        return 1;
    }

    Mood mood = MOOD_NEUTRAL;
    const char *dir_path = NULL;
    const char *file_path = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--mood") == 0 && i + 1 < argc) {
            mood = mood_parse(argv[i + 1]);
            i++;
        } else if (strcmp(argv[i], "--dir") == 0 && i + 1 < argc) {
            dir_path = argv[i + 1];
            i++;
        } else if (argv[i][0] != '-') {
            file_path = argv[i];
        }
    }

    if (!dir_path && !file_path) {
        printf("Error: Must specify either a MIDI file or --dir <directory>\n");
        return 1;
    }

    MarkovChain chain;
    markov_init(&chain);

    uint8_t first_pitch = 0;
    int success;

    if (dir_path) {
        success = train_from_directory(dir_path, &chain, &first_pitch);
    } else {
        printf("Training from file: %s\n", file_path);
        success = train_from_file(file_path, &chain, &first_pitch);
    }

    if (success <= 0) {
        printf("No notes found to train on\n");
        return 1;
    }

    markov_build_probs(&chain);

    if (mood != MOOD_NEUTRAL) {
        const char *mood_names[] = {"neutral", "sad", "happy", "energetic"};
        printf("Applying mood filter: %s\n", mood_names[mood]);
    }
    mood_apply(&chain, mood);

    printf("\n");

    srand(time(NULL));

    // Generate sequence
    uint8_t generated[32];
    markov_generate(&chain, first_pitch, generated, 32);

    printf("Generated sequence (starting from pitch %d):\n", first_pitch);
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
