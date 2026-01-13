#include <stdio.h>
#include "midi.h"

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

    printf("Parsed %d notes (ticks_per_beat: %d)\n", song.count, song.ticks_per_beat);

    // Test print first 10 notes as a sample
    int limit = song.count < 10 ? song.count : 10;
    for (int i = 0; i < limit; i++) {
        Note *n = &song.notes[i];
        printf("  pitch=%3d vel=%3d start=%6d dur=%4d\n",
               n->pitch, n->velocity, n->start, n->duration);
    }

    return 0;
}
