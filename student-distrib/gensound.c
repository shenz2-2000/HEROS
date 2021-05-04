#include "gensound.h"
#include "lib.h"
#include "sys_call.h"
#include "rtc.h"

// song_idx = 0
int32_t winxp_boot_freq[] = {622, 311, 466, 415, 622, 466};
int32_t winxp_boot_duration[] = {150, 107, 321, 428, 214, 856};
int32_t winxp_boot_length = 6;

// song_idx = 1
int32_t win10_alarm_freq[] = {587, 294, 440};
int32_t win10_alarm_duration[] = {54, 54, 214};
int32_t win10_alarm_length = 3;

// song_idx = 2
int32_t win93_halt_freq[] = {784, 587, 554, 392};
int32_t win93_halt_duration[] = {27, 54, 54, 107};
int32_t win93_halt_length = 4;

/**
 * gensound
 * Description: Play sound using built in speaker
 * Input: freq -- number of frequence
 *        time_in_ms -- time in unit ms
 * Output: always 0
 * Side effect: make sound
 */
//int32_t gensound(uint32_t freq, uint32_t time_in_ms) {
//    if (time_in_ms == 0) return 0;
//
//    sys_play_sound(freq);
//    sleep(time_in_ms);
//    sys_nosound();
//
//    return 0;
//}

/**
 * play_musics
 * Description: Play a music
 * Input: song - the song struct
 * Output: always 0
 * Side effect: play music
 */
//int32_t play_music(int32_t *freq, int32_t *duration, int32_t length) {
//    int i;
//    for (i = 0; i < length; i++) {
//        gensound(freq[i], duration[i]);
//    }
//    return 0;
//}

//int32_t play_song(int32_t song_idx) {
//    switch (song_idx) {
//        case 0:
//            play_music(winxp_boot_freq, winxp_boot_duration, winxp_boot_length);
//            break;
//        case 1:
//            play_music(win10_alarm_freq, win10_alarm_duration, win10_alarm_length);
//            break;
//        case 2:
//            play_music(win93_halt_freq, win93_halt_duration, win93_halt_length);
//            break;
//        default: break;
//    }
//    return 0;
//}
