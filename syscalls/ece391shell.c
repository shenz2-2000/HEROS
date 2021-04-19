#include <stdint.h>

#include "ece391support.h"
#include "ece391syscall.h"

#define BUFSIZE 1024

int32_t win10_alarm_freq[] = {587, 294, 440};
int32_t win10_alarm_duration[] = {54, 54, 214};


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

int main ()
{
    int i;
    int32_t cnt, rval;
    uint8_t buf[BUFSIZE];
    ece391_fdputs (1, (uint8_t*)"Starting 391 Shell\n");

    while (1) {
        ece391_fdputs (1, (uint8_t*)"391OS> ");
	if (-1 == (cnt = ece391_read (0, buf, BUFSIZE-1))) {
	    ece391_fdputs (1, (uint8_t*)"read from keyboard failed\n");
        for (i=0; i<3; i++) beep(win10_alarm_freq[i], win10_alarm_duration[i] / 32);
	    return 3;
	}
	if (cnt > 0 && '\n' == buf[cnt - 1])
	    cnt--;
	buf[cnt] = '\0';
	if (0 == ece391_strcmp (buf, (uint8_t*)"exit"))
	    return 0;
	if ('\0' == buf[0])
	    continue;
	rval = ece391_execute (buf);
	if (-1 == rval) {
        ece391_fdputs(1, (uint8_t *) "no such command\n");
        for (i = 0; i < 3; i++) beep(win10_alarm_freq[i], win10_alarm_duration[i] / 32);
    } else if (256 == rval) {
        ece391_fdputs(1, (uint8_t *) "program terminated by exception\n");
        for (i=0; i<3; i++) beep(win10_alarm_freq[i], win10_alarm_duration[i] / 32);
    } else if (0 != rval) {
        ece391_fdputs(1, (uint8_t *) "program terminated abnormally\n");
        for (i=0; i<3; i++) beep(win10_alarm_freq[i], win10_alarm_duration[i] / 32);
    }
    }
}

