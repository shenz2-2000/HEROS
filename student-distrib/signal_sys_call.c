//
// Created by BOURNE on 2021/4/16.
//

#include "signal_sys_call.h"
#include "process.h"
#include "lib.h"

signal_handler default_handler[SIGNAL_NUM]; // Store the default signal handler

/*
 * sys_set_handler
 *   DESCRIPTION: System Call: set_handler
 *   INPUTS: signum -- the index of signal handler that want to change
 *           handler_address -- the target signal handler function pointer
 *   OUTPUTS: 0 if success
 *   RETURN VALUE: none
 *   SIDE EFFECTS: no
 */
int32_t sys_set_handler(int32_t signum, void* handler_address) {
    if (signum >= SIGNAL_NUM || signum<0) {
        printf("Invalid input for sys_set_handle\n");
        return -1;
    }
    // Set function pointer of signal handler corresponding to input para
    int32_t flags;
    cli_and_save(flags);
    get_cur_process()->signals = (handler_address==NULL)?default_handler[signum]:handler_address;
    restore_flags(flags);
    return 0;
}

/*
 * signal_init
 *   DESCRIPTION: do the initialization for the default struct
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: initialize the signal struct
 */

void signal_init() {
    default_handler[0]=sig_div_zero_default;
    default_handler[1]=sig_seg_default;
    default_handler[2]=sig_interrupt_default;
    default_handler[3]=sig_alarm_default;
    default_handler[4]=sig_user1_default;
}

/*
 * task_signal_init
 *   DESCRIPTION: do the initialization for the signal struct
 *   INPUTS: signal_array -- the struct to be initialized
 *   OUTPUTS: none
 *   RETURN VALUE: 0 if success
 *   SIDE EFFECTS: initialize the signal struct
 */
int32_t task_signal_init(signal_t* signal_array) {
    if (signal_array == NULL) {
        printf("FAIL to initialize signal struct");
        return -1;
    }
    int i;
    signal_array->signal_pending=signal_array->signal_masked=signal_array->alarm_time=0;
    for(i = 0; i < SIGNAL_NUM; ++i) signal_array->sig[i] = default_handler[i];
    return 0;
}
/*
 * signal_send
 *   DESCRIPTION: send signal to the current process
 *   INPUTS: signum -- the index of signal handler that want to change
 *   OUTPUTS: 0 if success
 *   RETURN VALUE: none
 *   SIDE EFFECTS: no
 */
int32_t signal_send(int32_t signum) {
    //TODO: Ignore INTERRRUPT and ALARM at this point
    if (signum >= SIGNAL_NUM || signum<0) {
        printf("Invalid input for signal_send\n");
        return -1;
    }
    // Set pending signal corresponding to input para
    int32_t flags;
    cli_and_save(flags);
    get_cur_process()->signals.signal_pending |= 1<<signum;
    restore_flags(flags);
    return 0;
}
/*
 * signal_mask
 *   DESCRIPTION: mask a certain signal
 *   INPUTS: signum -- the index of signal handler that want to change
 *   OUTPUTS: 0 if success
 *   RETURN VALUE: none
 *   SIDE EFFECTS: no
 */
int32_t signal_mask(int32_t signum) {
    if (signum >= SIGNAL_NUM || signum<0) {
        printf("Invalid input for signal_mask\n");
        return -1;
    }
    // Set masked signal corresponding to input para
    int32_t flags;
    cli_and_save(flags);
    get_cur_process()->signals.signal_masked |= 1<<signum;
    restore_flags(flags);
    return 0;
}

/*
 * signal_unmask
 *   DESCRIPTION: unmask a certain signal
 *   INPUTS: signum -- the index of signal handler that want to change
 *   OUTPUTS: 0 if success
 *   RETURN VALUE: none
 *   SIDE EFFECTS: no
 */
int32_t signal_unmask(int32_t signum) {
    if (signum >= SIGNAL_NUM || signum<0) {
        printf("Invalid input for signal_unmask\n");
        return -1;
    }
    // Set masked signal corresponding to input para
    int32_t flags;
    cli_and_save(flags);
    get_cur_process()->signals.signal_masked &= ~(1<<signum);
    restore_flags(flags);
    return 0;
}

/*
 * signal_restore_mask
 *   DESCRIPTION: Restore previous mask
 *   INPUTS: None
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: no
 */

void signal_restore_mask(void) {
    // restore the previous mask
    int32_t flags;
    cli_and_save(flags);
    get_cur_process()->signals.signal_masked = get_cur_process()->signals.previous_masked;
    restore_flags(flags);
}
// Signal Handler Starts here:

