/*
 * Reference
 * http://www.topherlee.com/software/pcm-tut-wavformat.html
 * http://soundfile.sapp.org/doc/WaveFormat/
 * http://www.topherlee.com/software/pcm-tut-wavformat.html
 */

#include "sound_card.h"
#include "wav_player.h"
#include "lib.h"
#include "sys_call.h"

const uint8_t *wav_file_list[] = {"macOS_startup.wav"};

wave_file_struct* wave_file_parse() {}

int32_t play_wav(int32_t file_num) {
    // open the wav file
    int32_t dsp_fd, wav_fd;
    // wav_fd = sys_open()
}

