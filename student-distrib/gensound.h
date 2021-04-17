#ifndef _GENSOUND_H
#define _GENSOUND_H

#include "lib.h"

int32_t gensound(uint32_t freq, uint32_t time_in_ms);
int32_t play_music(int32_t *freq, int32_t *duration, int32_t length);
int32_t play_song(int32_t song_idx);

#endif
