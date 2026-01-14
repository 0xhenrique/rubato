#ifndef MIDI_H
#define MIDI_H

#include <stdint.h>

#define MAX_NOTES 10000

typedef struct {
    uint8_t pitch;      // 0-127 (60 = middle C)
    uint8_t velocity;   // 0-127
    uint32_t start;     // in ticks
    uint32_t duration;  // in ticks
} Note;

typedef struct {
    Note notes[MAX_NOTES];
    int count;
    uint16_t ticks_per_beat;
} Song;

int midi_parse(const char *filename, Song *song);
int midi_write(const char *filename, uint8_t *pitches, int count);

#endif
