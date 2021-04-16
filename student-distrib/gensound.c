#include "gensound.h"
#include "lib.h"
#include "sys_call.h"

int32_t gensound(uint32_t freq, uint32_t time_in_ms) {
    if (time_in_ms == 0) return 0;

    sys_play_sound(freq);
    sleep(time_in_ms);
    sys_nosound();

    return 0;
}
