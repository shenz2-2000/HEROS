//
// Created by BOURNE on 2021/4/16.
//

#include "signal_sys_call.h"
#include "x86_desc.h"
#include "process.h"
#include "sys_call.h"
#include "idt.h"
#include "lib.h"
extern void user_handler_helper(int32_t signum, signal_handler handler , hw_context *hw_context_addr);



/*
 * check_signal
 *   DESCRIPTION: check the current process's signal
 *   INPUTS: hw - hard_ware context
 *   OUTPUTS: None
 *   RETURN VALUE: None
 *   SIDE EFFECTS: None
 */
ASMLINKAGE void check_signal(hw_context hw){

    // TODO: something wrong with this line
    if (hw.cs != USER_CS){
        // if return context is not user space, do nothing
        return;
    }

    //pcb_t * cur_process;
    int32_t signal_idx;
    int32_t eflag;
    uint32_t cur_signal;


    // forbid interrupt
    cli_and_save(eflag);


    //cur_process = get_cur_process();
    cur_signal = (get_cur_process()->signals.signal_pending) & (~get_cur_process()->signals.signal_masked);
    if(cur_signal == 0) {
        restore_flags(eflag);
        return;
    }

    for(signal_idx = 0; signal_idx < SIGNAL_NUM; signal_idx ++){
        if(  ( (cur_signal >> signal_idx) & 0x00000001 )  == 1 ) break;
    }

    // reset the signals
    get_cur_process()->signals.previous_masked = get_cur_process()->signals.signal_masked;
    get_cur_process()->signals.signal_pending = 0;
    get_cur_process()->signals.signal_masked = MASK_ALL;

    if( get_cur_process()->signals.sig[signal_idx] == default_handler[signal_idx] ) {

        default_handler[signal_idx]();
        // restore the mask
        restore_signal();

    }

    else{
        // now we need to setup the stack frame
        user_handler_helper(signal_idx, get_cur_process()->signals.sig[signal_idx], &hw);
    }

    restore_flags(eflag);

}



/*
 * restore_signal()
 *   DESCRIPTION: restore the signal
 *   INPUTS: None
 *   OUTPUTS: None
 *   RETURN VALUE: None
 *   SIDE EFFECTS: None
 */
void restore_signal(){
    int32_t eflag_for_exec;

    // clear IF
    cli_and_save(eflag_for_exec);

    get_cur_process()->signals.signal_masked = get_cur_process()->signals.previous_masked;

    restore_flags(eflag_for_exec);
}



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
    if (handler_address == NULL) {
        get_cur_process()->signals.sig[signum] = default_handler[signum];
    } else {
        get_cur_process()->signals.sig[signum] = (signal_handler) handler_address;
    }
//    get_cur_process()->signals.sig[signum] = (handler_address==NULL)?default_handler[signum]:(signal_handler)handler_address;
    restore_flags(flags);
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
    if (signum == 2 || signum == 3) {// interrupt or alarm
        if (get_showing_task() != NULL)
            get_showing_task()->signals.signal_pending |= 1<<signum;
    } else {
        get_cur_process()->signals.signal_pending |= 1<<signum;
    }
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
 * sig_div_zero_default
 *   DESCRIPTION: Default handler for divide by zero
 *   INPUTS: None
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: no
 */
int32_t sig_div_zero_default() {
    int32_t flags;
    printf("Successfully Get in\n");
    cli_and_save(flags);
    system_halt(256);
    restore_flags(flags);

    return -1; // It should not return
}
/*
 * sig_seg_default
 *   DESCRIPTION: Default handler for segmentation fault
 *   INPUTS: None
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: no
 */
int32_t sig_seg_default() {
    int32_t flags;
    cli_and_save(flags);
    system_halt(256);
    restore_flags(flags);

    return -1; // It should not return
}


/*
 * sig_interrupt_default
 *   DESCRIPTION: Default handler for interrupt
 *   INPUTS: None
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: no
 */
int32_t sig_interrupt_default() {
    int32_t flags;
    cli_and_save(flags);
    system_halt(0);
    restore_flags(flags);

    return -1; // It should not return
}

/*
 * sig_alarm_default
 *   DESCRIPTION: Default handler for alarm
 *   INPUTS: None
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: no
 */
int32_t sig_alarm_default() {
    return 0;
}

/*
 * sig_user1_default
 *   DESCRIPTION: Default handler for user1
 *   INPUTS: None
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: no
 */
int32_t sig_user1_default() {
    return 0;
}


/*
 * test_handler
 *   DESCRIPTION: handler used to test set_handler
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */

int32_t test_handler() {
    int32_t flags;
    printf("Set handler success\n");
    cli_and_save(flags);
    system_halt(256);
    restore_flags(flags);

    return -1; // It should not return
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
int32_t task_signal_init(signal_struct_t* signal_array) {
    if (signal_array == NULL) {
        printf("FAIL to initialize signal struct");
        return -1;
    }
    int i;
    signal_array->signal_pending=signal_array->signal_masked=signal_array->alarm_time=0;
    for(i = 0; i < SIGNAL_NUM; ++i) signal_array->sig[i] = default_handler[i];
    return 0;
}
