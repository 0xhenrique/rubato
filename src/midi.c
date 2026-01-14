#include "midi.h"
#include <stdio.h>
#include <string.h>

static uint16_t read16(FILE *f) {
    uint8_t buf[2];
    fread(buf, 1, 2, f);
    return (buf[0] << 8) | buf[1];
}

static uint32_t read32(FILE *f) {
    uint8_t buf[4];
    fread(buf, 1, 4, f);
    return (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
}

static uint32_t read_varlen(FILE *f) {
    uint32_t value = 0;
    uint8_t byte;
    do {
        fread(&byte, 1, 1, f);
        value = (value << 7) | (byte & 0x7F);
    } while (byte & 0x80);
    return value;
}

int midi_parse(const char *filename, Song *song) {
    FILE *f = fopen(filename, "rb");
    if (!f) return -1;

    song->count = 0;

    // Read header chunk "MThd"
    char header[4];
    fread(header, 1, 4, f);
    if (memcmp(header, "MThd", 4) != 0) {
        fclose(f);
        return -1;
    }

    read32(f); // header length (always 6)
    read16(f); // format (0, 1, or 2)
    uint16_t num_tracks = read16(f);
    song->ticks_per_beat = read16(f);

    // Track which notes are currently "on" (used for calculating duration)
    uint32_t note_on_time[128] = {0};
    uint8_t note_on_velocity[128] = {0};
    int note_on_active[128] = {0};

    // Parse tracks
    for (int t = 0; t < num_tracks; t++) {
        char chunk_type[4];
        fread(chunk_type, 1, 4, f);
        uint32_t chunk_len = read32(f);

        if (memcmp(chunk_type, "MTrk", 4) != 0) {
            fseek(f, chunk_len, SEEK_CUR); // skip unknown chunk
            continue;
        }

        long track_end = ftell(f) + chunk_len;
        uint32_t current_time = 0;
        uint8_t running_status = 0;

        while (ftell(f) < track_end) {
            uint32_t delta = read_varlen(f);
            current_time += delta;

            uint8_t status;
            fread(&status, 1, 1, f);

            // Handle running status
            if (status < 0x80) {
                fseek(f, -1, SEEK_CUR);
                status = running_status;
            } else {
                running_status = status;
            }

            uint8_t type = status & 0xF0;

            if (type == 0x90 || type == 0x80) {
                // Note On or Note Off
                uint8_t pitch, velocity;
                fread(&pitch, 1, 1, f);
                fread(&velocity, 1, 1, f);

                // Note On with velocity 0 = Note Off
                int is_note_on = (type == 0x90 && velocity > 0);

                if (is_note_on) {
                    note_on_time[pitch] = current_time;
                    note_on_velocity[pitch] = velocity;
                    note_on_active[pitch] = 1;
                } else if (note_on_active[pitch]) {
                    // Note Off - create the note
                    if (song->count < MAX_NOTES) {
                        song->notes[song->count].pitch = pitch;
                        song->notes[song->count].velocity = note_on_velocity[pitch];
                        song->notes[song->count].start = note_on_time[pitch];
                        song->notes[song->count].duration = current_time - note_on_time[pitch];
                        song->count++;
                    }
                    note_on_active[pitch] = 0;
                }
            } else if (type == 0xA0 || type == 0xB0 || type == 0xE0) {
                // Aftertouch, Control Change, Pitch Bend - skip 2 bytes
                fseek(f, 2, SEEK_CUR);
            } else if (type == 0xC0 || type == 0xD0) {
                // Program Change, Channel Pressure - skip 1 byte
                fseek(f, 1, SEEK_CUR);
            } else if (status == 0xFF) {
                // Meta event
                uint8_t meta_type;
                fread(&meta_type, 1, 1, f);
                uint32_t len = read_varlen(f);
                fseek(f, len, SEEK_CUR);
            } else if (status == 0xF0 || status == 0xF7) {
                // SysEx
                uint32_t len = read_varlen(f);
                fseek(f, len, SEEK_CUR);
            }
        }
    }

    fclose(f);
    return 0;
}

static void write16(FILE *f, uint16_t val) {
    uint8_t buf[2] = { val >> 8, val & 0xFF };
    fwrite(buf, 1, 2, f);
}

static void write32(FILE *f, uint32_t val) {
    uint8_t buf[4] = { val >> 24, (val >> 16) & 0xFF, (val >> 8) & 0xFF, val & 0xFF };
    fwrite(buf, 1, 4, f);
}

int midi_write(const char *filename, uint8_t *pitches, int count) {
    FILE *f = fopen(filename, "wb");
    if (!f) return -1;

    uint16_t ticks_per_beat = 480;
    uint16_t note_duration = 240;  // half a beat
    uint8_t velocity = 100;

    // Calculate track data size
    // Each note: delta(1) + noteOn(3) + delta(1-2) + noteOff(3)
    // Plus end of track: delta(1) + meta(3)
    int track_size = 0;
    for (int i = 0; i < count; i++) {
        track_size += 1 + 3;  // delta 0 + note on
        track_size += (note_duration < 128 ? 1 : 2) + 3;  // delta + note off
    }
    track_size += 4;  // end of track

    // Header chunk
    fwrite("MThd", 1, 4, f);
    write32(f, 6);              // header length
    write16(f, 0);              // format 0
    write16(f, 1);              // 1 track
    write16(f, ticks_per_beat);

    // Track chunk
    fwrite("MTrk", 1, 4, f);
    write32(f, track_size);

    // Write notes
    for (int i = 0; i < count; i++) {
        // Note On (delta = 0)
        fputc(0x00, f);         // delta time
        fputc(0x90, f);         // note on, channel 0
        fputc(pitches[i], f);
        fputc(velocity, f);

        // Note Off (delta = duration)
        if (note_duration < 128) {
            fputc(note_duration, f);
        } else {
            fputc(0x81, f);     // variable length: 240 = 0x81 0x70
            fputc(0x70, f);
        }
        fputc(0x80, f);         // note off, channel 0
        fputc(pitches[i], f);
        fputc(0, f);            // velocity
    }

    // End of track
    fputc(0x00, f);
    fputc(0xFF, f);
    fputc(0x2F, f);
    fputc(0x00, f);

    fclose(f);
    return 0;
}
