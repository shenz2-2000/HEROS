#ifndef _WAV_PLAYER_H
#define _wAV_PLAYER_H

#include "lib.h"

typedef struct wave_file_struct {
    /* The "RIFF" chunk descriptor */
    uint32_t chunkID;       // Contains the letters "RIFF" in ASCII form (0x52494646 big-endian form).
    uint32_t chunkSize;     // 36 + SubChunk2Size, or more precisely: 4 + (8 + SubChunk1Size) + (8 + SubChunk2Size)
    uint32_t format;        // Contains the letters "WAVE" (0x57415645 big-endian form).

    /* The "fmt" sub-chunk */
    uint32_t subchunk1ID;   // Contains the letters "fmt " (0x666d7420 big-endian form).
    uint32_t subchunk1Size; // 16 for PCM.  This is the size of the rest of the Subchunk which follows this number.
    uint16_t audioFormat;   // PCM = 1 (i.e. Linear quantization) Values other than 1 indicate some 
    uint16_t numChannels;   // Mono = 1, Stereo = 2, etc.
    uint32_t sampleRate;    // 8000, 44100, etc.
    uint32_t byteRate;      // SampleRate * NumChannels * BitsPerSample/8
    uint16_t blockAlign;    // NumChannels * BitsPerSample/8
    uint16_t bitsPerSample; // 8 bits = 8, 16 bits = 16, etc.

    /* The "data" sub-chunk */
    uint32_t subchunk2ID;   // Contains the letters "data" (0x64617461 big-endian form).
    uint32_t subchunk2Size; // NumSamples * NumChannels * BitsPerSample/8

    /* Data */
} wave_file_struct;

int32_t wave_file_parse(int32_t wav_fd, wave_file_struct *wav_file);
int32_t sound_card_init(int32_t dsp_fd, wave_file_struct *wav_file);
int32_t play_wav(int32_t file_num);

#endif
