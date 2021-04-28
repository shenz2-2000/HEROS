#include <stdint.h>
#include "ece391support.h"
#include "ece391syscall.h"

// music scores: frequency in hz and duration in ms
int32_t freq_default[] = {1047, 1319, 1047, 1319, 1047, 1319, 1047, 1319, 1047, 1319};
int32_t dura_default[] = {500, 500, 500, 500, 500, 500, 500, 500, 500, 500};

/**
 * @brief generate the sound at a given frequency for a given duration (approximately)
 * @param freq frequency in hz
 * @param dura duration in ms
 * @side_effect open rtc and change frequency
 */
void beep(uint32_t freq, uint32_t dura) {
    int i;
    uint8_t dum[] = "d";
    uint8_t buf[] = "rtc";
    uint32_t t = 32;
    // set the rtc
    int dev_rtc = ece391_open(buf);
    ece391_write(dev_rtc, &t, 4);
    if (dev_rtc == -1) {
        ece391_fdputs (1, (uint8_t*)"In beep(): fail to set RTC.\n");
        return;
    }
    // play the music
    ece391_play_sound(freq);
    for (i = 0; i < dura; i++) ece391_read(dev_rtc, dum, 1);
    ece391_nosound();
    ece391_close(dev_rtc);
}

int32_t main(){
//    int i;
//    for (i=0; i<10; i++) beep(freq_default[i], dura_default[i] / 32);
    int i;
    uint8_t dum[] = "d";
    uint8_t buf[] = "rtc";
    ece391_fdputs (1, (uint8_t*)"Konijiwaq!\n");
    int dev_rtc = ece391_open(buf);
    for (i = 0; i < 3; i++) ece391_read(dev_rtc, dum, 1);
    ece391_fdputs (1, (uint8_t*)"Fantastic BiBi!\n");
    for (i = 0; i < 3; i++) ece391_read(dev_rtc, dum, 1);

    return 0;
}
